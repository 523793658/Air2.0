#include "pi_vcall.h"

#include <Windows.h>
#include <sstream>
#include "v8-profiler.h"

#include <pi_lib.h>

#include "game_service/Manager/PiGameServiceManager.h"
#include "game_service/Win32/WindowsService.h"
#include "game_service/Settings/SettingsService.h"
#include "pi_vcall_fun.h"
#include "pi_converter.h"
#include "remote_object_freer.h"
#include "game_service\PiScriptingEngine\tasks\HttpRequestTask.h"
#include "game_tools/GameTools.h"
#include "game_service/PiScriptingEngine/backtrace.h"


using namespace v8;

#undef WCHAR_SIZE
#define WCHAR_SIZE		sizeof(wchar)

/* Windows has no snprintf, only _snprintf. */
#define snprintf _snprintf

bool gLog = true;
bool gCheckType = true;
FunItem gFunItem[MAX_FUN_NUM];
int gIndex = 1; // js中0表示未初始化,因此g_funItem[0]不使用

//invoke needed
const int MAX_SIZE = 1024;
const int MAX_SIZE_STR = 2048;

static wchar* EMPTY_STRING = (wchar*)pi_malloc0(sizeof(wchar));

/* 虚拟内存页面的字节数 */
#define MEM_PAGE_SIZE 4096

int JSWriteStringW(v8::Local<v8::Value> val, wchar* pCopyDest, int bufLen);
int JSWriteStringMB(v8::Local<v8::Value> val, char* pCopyDest, PiCodePage encoding, int bufLen);

/**
* wstr变str返回
* page代表char*的页码，如果为其他值，则直接使用
* 注：返回的指针需要用pi_free释放
*/
char*	pi_wstr_to_str_len(const wchar *wstr, uint len, PiCodePage page) {
	char *str = NULL;
	if (wstr != NULL) {
		int cp = get_code_page_win32(page);
		uint n = WideCharToMultiByte(cp, 0, wstr, len, NULL, 0, NULL, 0);
		str = (char*)pi_malloc(n + 1);
		if (str != NULL) {
			WideCharToMultiByte(cp, 0, wstr, len, str, n, NULL, 0);
			str[n] = '\0';
		}
	}
	return str;
}

/**
* str变wstr返回
* page代表char*的页码，如果为其他值，则直接使用
* 注：返回的指针需要用pi_free释放
*/
wchar*	pi_str_to_wstr_len(const char *str, uint len, PiCodePage page) {
	wchar *wstr = NULL;
	if (str != NULL) {
		int cp = get_code_page_win32(page);
		uint n = MultiByteToWideChar(cp, 0, str, len, NULL, 0);
		wstr = (wchar *)pi_malloc((n + 1) * sizeof(wchar));
		if (wstr != NULL) {
			MultiByteToWideChar(cp, 0, str, len, wstr, n);
			wstr[n] = L'\0';
		}
	}
	return wstr;
}

int getType(v8::Local<v8::Value> val)
{
	int type = ERROR_UNKNOW_ARG_TYPE;
	if(val->IsInt32() || val->IsNull() || val->IsUndefined() || val->IsBoolean()) {
		type = TypeInteger;
	} else if(val->IsUint32()) {
		type = TypeUint;
	} else if(val->IsNumber()) {
		type = TypeDouble;
	} else if(val->IsString()) {
		type = TypeString;
	} else if(val->IsObject()) {
		type = TypeObject;
	}
	return type;
}

bool checkType(v8::Local<v8::Value> val, int expectedType)
{
	if(val->IsNumber() || val->IsBoolean()) {
		if(expectedType > TypeDouble && expectedType != TypeBufferPointer)
			return false;
	} else if(val->IsNull() || val->IsUndefined()) {
		if(expectedType < TypeString && expectedType > TypeBufferContent)
			return false;
	} else if(val->IsString()) {
		if(expectedType < TypeString || expectedType > TypeStringWithCoding)
			return false;
	} else if(val->IsObject()) {
		if(expectedType < TypeObject || expectedType > TypeBufferContent)
			return false;
	} else {
		return false;
	}

	return true;
}

// 参数值缓冲：只处理字符串和ArrayBuffer
inline char* GetParamValueBuffer(Nan::NAN_METHOD_ARGS_TYPE info)
{
	int size = 0;
	char *buf = NULL;

	Local<Context> context = info.GetIsolate()->GetCurrentContext();
	for (int i = 2; i < info.Length(); i++)
	{
		v8::Local<v8::Value> arg = info[i];

		if (arg->IsArrayBuffer())
		{
			v8::Local<v8::ArrayBuffer> arrybuf = v8::Local<v8::ArrayBuffer>::Cast(arg);
			size += arrybuf->ByteLength();
		}
		else if (arg->IsArrayBufferView()) {
			v8::Local<v8::ArrayBufferView> bv = v8::Local<v8::ArrayBufferView>::Cast(arg);
			size += bv->ByteLength();
		}
		else if (arg->IsString())
		{
			size += sizeof(wchar) * (arg->ToString(context).ToLocalChecked()->Length() + 1);
		}
	}

	if (size > 0)
		buf = (char*)pi_malloc0(size);
	return buf;
}


/* 带越界检查的free */
static void check_bound_free(void *p)
{
	/* 找到要释放的实际页面块 */
	p = (char *)p - (size_t)p % MEM_PAGE_SIZE;  

	if(NULL != p)
		VirtualFree(p, 0, MEM_RELEASE);
}

// 校验字符串参数是否过长.只能针对unicode参数的情形.

inline bool CheckBuffer(char** outBuf, Nan::NAN_METHOD_ARGS_TYPE info)
{
    //计算字符串占用的空间
    int strLen = 0;
	bool newFlag = false;
    for(int i=1; i<info.Length(); i++)
    {
        if(info[i]->IsString())
        {
            std::wstring t = V8ToWString(info[i]);
            strLen += t.size() + 1;
        }
    }
    if(strLen * sizeof(uint16_t) > MAX_SIZE_STR)
    {
		newFlag = true;
        //如果字符串太大,则在堆上分配
        *outBuf = new char[strLen * sizeof(wchar)];
    }
	return newFlag;
}

// 获取JSArrayBuffer或者JSArrayBufferView的buffer首地址&长度.
void* GetArrayBuffer(v8::Local<v8::Value> var, unsigned int* arrayBufSize)
{
	// blink 不再支持直接转换v8 value-> blink::WebArrayBuffer!.
	if( var->IsArrayBuffer() ) {

		//ThrowException(__FUNCTION__, "GetArrayBuffer failed, must use array buffer's view");
		return NULL;
	} else if(var->IsArrayBufferView()) {

		Local<ArrayBufferView> bv = Local<ArrayBufferView>::Cast(var);
		if (arrayBufSize) {
			*arrayBufSize = bv->ByteLength();
		}
		void* ret = bv->Buffer()->GetContents().Data();

		return ret;
	}
	else
		return NULL;
}

/**
* 获得buffer
* @param  es
* @param  obj
* @param  jsValueBufPointer
* @param  jsValueTotalSize
* @return
*/
//Try to get the address of js buffer or raw buffer(int value)
//obuf: 不要传入null or undefined, 否则会引发js异常(null is not an object)
inline bool tryGetBuffer(v8::Local<v8::Value> obuf, char** ppBuf, unsigned int* pByteLen)
{
    bool r = true;
	v8::Local<v8::Context> context = v8::Isolate::GetCurrent()->GetCurrentContext();
    if( obuf->IsInt32() || obuf->IsNumber() ){
        *ppBuf = (char*)(unsigned)obuf->Int32Value(context).FromJust();
        if (*ppBuf == NULL)
            *ppBuf = (char*)(unsigned)obuf->NumberValue(context).FromJust();
        if(pByteLen)
            *pByteLen = 0xffffffff;
    }
    else{
        *ppBuf = (char*)GetArrayBuffer(obuf, pByteLen);
        r = (*ppBuf != NULL);
    }
    return r;
}

// 打印类型的详细信息
void PrintType(v8::Local<v8::Value> val)
{
	std::string  str = V8ToString(val);
	printf("tostring()=%s, object=%d, string=%d, bool=%d, array=%d, external=%d, int32=%d, uint32=%d, number=%d, null=%d, function=%d, date=%d, undefined=%d\n",
		str.c_str(),
		val->IsObject(),
		val->IsString(),
		val->IsBoolean(),
		-1,//IsArray
		-1,//isExternal
		val->IsInt32(),
		val->IsUint32(),
		val->IsNumber(),
		val->IsNull(),
		val->IsFunction(),
		-1,//isDate()
		val->IsUndefined()
		);
	printf("\n");
}

/*
 * WriteBufferNumber, common part of writing numerical numbers(signed/unsigned byte/int16/int/int64 & float/double)
 * used by WriteBuffer && UpdateObject(to buffer);
 * return written bytes, 0 if fails(buffer overflow), < 0 is error code
 **/
inline int writeBufferNumber( char* buf, unsigned int byteLen, unsigned int offset, v8::Local<v8::Value> Val, int type)
{
	int writeBytes = 0;
	v8::Local<v8::Context> context = v8::Isolate::GetCurrent()->GetCurrentContext();
	switch(type)
	{
	case TypeInteger:
	case TypeUint:
	case TypeBoolean:
		if(offset+4 <= byteLen)
		{
			(*(int*)(buf+offset)) = Val->Int32Value(context).FromJust();
			writeBytes = 4;
		}
		break;
	case TypeByte:
		if(offset+1 <= byteLen)
		{
			buf[offset] = static_cast<char>(Val->Int32Value(context).FromJust());
			writeBytes = 1;
		}
		break;
	case TypeShort:
		if(offset+2 <= byteLen)
		{
			(*(int16*)(buf+offset)) = static_cast<int16>(Val->Int32Value(context).FromJust());
			writeBytes = 2;
		}
		break;
	case TypeLong:
		if(offset+2 <= byteLen)
		{
			(*(int64*)(buf+offset)) = (int64)Val->NumberValue(context).FromJust();
			writeBytes = 8;
		}
		break;
	case TypeDouble:
		if(offset+8 <= byteLen)
		{
			(*(double*)(buf+offset)) = static_cast<double>(Val->NumberValue(context).FromJust());
			writeBytes = 8;
		}
		break;
	case TypeFloat:
		if(offset+4 <= byteLen)
		{
			(*(float*)(buf+offset)) = static_cast<float>(Val->NumberValue(context).FromJust());
			writeBytes = 4;
		}
		break;
	case TypeObject:
		if(offset+8 <= byteLen)
		{
			(*(int64*)(buf+offset)) = Val->IntegerValue(context).FromJust();
			writeBytes = 8;
		}
		break;
	default:
		{
			//char buf[300];
			//sprintf_s(buf,300,"WriteBufferNumber error, unknown type=%d",type);
			//ThrowException("WriteBufferNumber",buf);
			writeBytes = ERROR_UNKNOW_ARG_TYPE;
		}
		break;
	}

	return writeBytes;
}

/*
* WriteBufferBuffer, common part of writing buffer content
* used by WriteBuffer && UpdateObject(to buffer);
* return written bytes, 0 if fails
**/
inline int writeBufferBuffer( char* buf, unsigned int byteLen, unsigned int offset, v8::Local<v8::Value> SrcBuf, int srcOffset, int srcLen)
{
    unsigned int srcByteLen = 0;
    char* srcBuf;
    if(!(char*)tryGetBuffer( SrcBuf, &srcBuf, &srcByteLen))
    {
        //ThrowException("writeBufferBuffer","WriteBufferBuffer error, invalid buffer");
        printf("writeBufferBuffer: %s","WriteBufferBuffer error, invalid buffer");
        return 0;
    }

    if(srcOffset >= 0 && (unsigned int)(srcOffset + srcLen) <= srcByteLen && offset + srcLen <= byteLen)
    {
        memcpy(buf+offset, srcBuf+srcOffset, srcLen);
        return srcLen;
    }
    return 0;
}

/*
* WriteBufferBufferPointer, common part of writing buffer pointer
* used by WriteBuffer && UpdateObject(to buffer);
* return written bytes, 0 if fails
**/
inline int writeBufferBufferPointer( char* buf, unsigned int byteLen, unsigned int offset, v8::Local<v8::Value> SrcBuf, int srcOffset)
{
    int writeBytes = 0;
    if(offset+4 <= byteLen)
    {
        //允许传入空缓冲区,可以传入null/undefined/0
        if(SrcBuf->IsNull() || SrcBuf->IsUndefined())
        {
            //此时如果调用TryGetBuffer会造成js异常(null is not an object)
            writeBytes = 4;
            *(int*)(buf+offset) = 0;
        }
        else
        {
            char* obuf;
            if(tryGetBuffer( SrcBuf, &obuf, NULL))
            {
                writeBytes = 4;
                *(int*)(buf+offset) = (int)(obuf+srcOffset);
            }
            else
            {
                writeBytes = ERROR_ARG_INVALID_BUFFER;
            }
        }
    }
    return writeBytes;
}

Local<v8::Value> UpdateObj(Nan::NAN_METHOD_ARGS_TYPE info, Local<Value> obj, Local<Array> descArr, Local<Value> objBuf, Local<Value> off, bool bFromBuffer = true)
{
	int len = descArr->Length();
	//char info[300];

	if (!obj->IsObject() || len < 2)
	{
// 		sprintf_s(info, 300, "UpdateObj arg error, obj is obj=%d, descArr len=%d", obj->IsObject(), len);
// 		ThrowException("UpdateObj", info);
		return v8::Undefined(info.GetIsolate());
	}
	const int MaxPropLen = 1024;
	wchar propBuf[MaxPropLen];//单个属性不能超过1023字符数
	unsigned int offset = 0;
	unsigned int byteLen;
	char* buf;
	Local<Context> context = info.GetIsolate()->GetCurrentContext();
	offset += (unsigned)off->Int32Value(context).FromJust();
	if (!tryGetBuffer(objBuf, &buf, &byteLen))
	{
		/*ThrowException("UpdateObj", "UpdateObj error, invalid buffer");*/
		return v8::Undefined(info.GetIsolate());
	}
	for (int i = 1; i < len; i += 2)//start from 1 to avoid len is odd
	{
		Local<Value> prop = descArr->Get(context, i - 1).ToLocalChecked();//descArrV.get(ES, i-1);
		JSWriteStringW(prop, propBuf, MaxPropLen*sizeof(wchar));
		int type = (descArr->Get(i))->Int32Value(context).FromJust();  //es, descArr, i);
		if (bFromBuffer)
		{
			Local<Value> r = readBufferCore(false, info, objBuf, &offset, type);
			if (!r->IsUndefined())
			{
				Local<Object> _obj = Local<Object>::Cast(obj);
				_obj->Set(String::NewFromTwoByte(info.GetIsolate(), (uint16_t*)propBuf), r);
			}
			else
			{
				//sprintf_s(info, 300, "UpdateObj read error, offset=%d, type=%s", offset, GetTypeStr(type));
				//ThrowException("UpdateObj", info);
				return v8::Undefined(info.GetIsolate());
			}
		}
		else
		{
			//JSC::JSValue Val = JSGetW(es, obj, propBuf);
			Local<Object> _obj = v8::Local<Object>::Cast(obj);
			Local<Value> Val = _obj->Get(v8::String::NewFromTwoByte(info.GetIsolate(), (uint16_t*)propBuf));
			int writeBytes = 0;
			if (type == TypeBufferContent)
			{
				writeBytes = writeBufferBuffer(buf, byteLen, offset, Val, 0, -1);
			}
			else if (type == TypeBufferPointer)
			{
				writeBytes = writeBufferBufferPointer(buf, byteLen, offset, Val, 0);
			}
			else if (type == TypeString || type == TypeStringAnsi || type == TypeUTF8)
			{
				writeBytes = writeBufferString(buf, byteLen, offset, Val, type);
			}
			else
			{
				writeBytes = writeBufferNumber(buf, byteLen, offset, Val, type);
			}
			offset += writeBytes;
		}
	}
	return obj;
}

/**
* write \0 if possible
* 将|val|的字符串写入到|pCopyDest|
* 编码为unicode
* 成功返回写入的字节数,包括'\0', 否则返回0
* 返回值意义同pi_wstr_to_str_buf
*/
int JSWriteStringW(v8::Local<v8::Value> val, wchar* pCopyDest, int bufLen)
{
	std::wstring str = V8ToWString(val);
	int len = str.length();
	int writeBytes = (len + 1) * WCHAR_SIZE;//实际写入的字节数
	if(bufLen < 0 || bufLen >= writeBytes)
	{
		memcpy(pCopyDest, str.data(), writeBytes);
		if(writeBytes + (int)WCHAR_SIZE <= bufLen){
			writeBytes += WCHAR_SIZE;
			pCopyDest[len] = 0;
		}
		return writeBytes;
	}
	return 0;
}

/**
 * write \0 if possible
 * 将|val|的字符串写入到|pCopyDest|
 * 通过|encoding|进行编码
 * 成功返回写入的字节数,包括'\0', 否则返回0
 * 返回值意义同pi_wstr_to_str_buf
*/
int JSWriteStringMB(v8::Local<v8::Value> val, char* pCopyDest, PiCodePage encoding, int bufLen)
{
	std::wstring str = V8ToWString(val);
	int len = str.length();
	if(bufLen < 0)
		bufLen = 0x7fffffff;
	if(len==0)
	{
		if(bufLen)
		{
			pCopyDest[0] = 0;
			return 1;
		}
		else	
			return 0;
	}

	return pi_wstr_to_str_buf(pCopyDest, bufLen, (wchar*)str.data(), str.length(), encoding);
}

/*
* WriteBufferString, common part of writing string content
* used by WriteBuffer && UpdateObject(to buffer);
* return written bytes
**/
inline int writeBufferString( char* buf, unsigned int byteLen, unsigned int offset,v8::Local<v8::Value> Val, int type)
{
    int writeBytes;
    if(type == TypeString)
    {
        wchar* str = (wchar*)(buf+offset);
        writeBytes = JSWriteStringW(Val, str, byteLen - offset);
    }
    else
    {
        char* str = buf+offset;
        PiCodePage encoding = type == TypeStringAnsi ? PI_CP_ACP : PI_CP_UTF8;
        writeBytes = JSWriteStringMB( Val, str, encoding, byteLen - offset);
    }
    return writeBytes;
}

//add to readBufferCore中ConvertToV8方法
v8::Local<v8::Value> ConverterToV8_int32_t(v8::Isolate* isolate, int32_t val) {
  return MATE_INTEGER_NEW(isolate, val);
}
v8::Local<v8::Value> ConverterToV8_uint32_t(v8::Isolate* isolate, uint32_t val) {
  return MATE_INTEGER_NEW_UNSIGNED(isolate, val);
}


/*
 * ReadBufferCore, used by ReadBuffer && UpdateObject(from buffer);
 * bFromReadBuffer, invoked by ReadBuffer?
 **/
v8::Local<v8::Value> readBufferCore(bool bFromReadBuffer, Nan::NAN_METHOD_ARGS_TYPE info, v8::Local<v8::Value> ibuf, unsigned int* pOffset, int type)
{
	unsigned int byteLen;
	char* buf;
	if(!tryGetBuffer( ibuf, &buf, &byteLen)) {
		//ThrowException("readBufferCore","Read error, invalid buffer");
		return v8::Undefined(info.GetIsolate());
	}

	if(!buf) {
		//ThrowException("readBufferCore","Read error, null buffer ptr");
		return v8::Undefined(info.GetIsolate());
	}

	unsigned int offset = *pOffset;
	v8::Local<v8::Value> r = v8::Undefined(info.GetIsolate());
	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
	switch(type) {
	case TypeInteger:
	case TypeBoolean:
		if(offset+4 <= byteLen){
			r = ConverterToV8_int32_t(info.GetIsolate(),(*(int*)(buf+offset)));
			//r = jsInt(*(int*)(buf+offset));
			*pOffset = offset + 4;
		}
		break;
	case TypeUint:
		if(offset+4 <= byteLen){
			r = ConverterToV8_uint32_t(info.GetIsolate(),(*(unsigned int*)(buf+offset)));
			//r = JSNUMBER(*(unsigned int*)(buf+offset));//uint超出了int的范围,所以直接构造为number
			*pOffset = offset + 4;
		}
		break;
	case TypeByte:
		if(offset+1 <= byteLen){
			r = v8::Int32::New(info.GetIsolate(),buf[offset]);
			*pOffset = offset + 1;
		}
		break;

	case TypeUbyte:
		if(offset+1 <= byteLen){
			r = v8::Int32::New(info.GetIsolate(),(uint8)buf[offset]);
			*pOffset = offset + 1;
		}
		break;
	case TypeShort:
		if(offset+2 <= byteLen){
			r = v8::Int32::New(info.GetIsolate(),*(int16*)(buf + offset));
			*pOffset = offset + 2;
		}
		break;
	case TypeUshort:
		if(offset+2 <= byteLen){
			r = v8::Int32::New(info.GetIsolate(),*(uint16*)(buf + offset));
			*pOffset = offset + 2;
		}
		break;
	case TypeLong:
		if(offset+8 <= byteLen){
			r = v8::Number::New(info.GetIsolate(),*(int64*)(buf + offset));
			*pOffset = offset + 8;
		}
		break;
	case TypeUlong:
		if(offset+8 <= byteLen){
			r = v8::Number::New(info.GetIsolate(),*(uint64*)(buf + offset));
			*pOffset = offset + 8;
		}
		break;
	case TypeDouble:
		if(offset+8 <= byteLen){
			r = v8::Number::New(info.GetIsolate(),*(double*)(buf + offset));
			*pOffset = offset + 8;
		}
		break;
	case TypeFloat:
		if(offset+4 <= byteLen){
			r = v8::Number::New(info.GetIsolate(),*(float*)(buf + offset));
			*pOffset = offset + 4;
		}
		break;
	case TypeObject:
		if(offset+8 <= byteLen){
			r = v8::Number::New(info.GetIsolate(),*(int64*)(buf + offset));
			*pOffset = offset + 8;
		}
		break;
	case TypeString:
		{
			wchar* str = (wchar*)(buf+offset);
			int readBytes = bFromReadBuffer && info.Length() > 3 ? info[3]->Int32Value(context).FromJust() : -1;
			if(readBytes <0 || offset + readBytes <= byteLen)
			{
				if(readBytes >= 0){
					r = v8::String::NewFromTwoByte(info.GetIsolate(),(uint16_t*)str,v8::String::kNormalString, readBytes / 2);
					//JSC::JSValue strval = JSC::jsString(ES, JSC::UString(str, readBytes/2));
					*pOffset = offset + readBytes;
					//r = JSC::JSValue::encode(strval);
				}
				else{
					v8::Local<v8::String> ustr = v8::String::NewFromTwoByte(info.GetIsolate(),(uint16_t*)str);
					readBytes = (ustr->Length()+1)*sizeof(wchar);
					if(offset + readBytes <= byteLen)
					{
						*pOffset = offset + readBytes;
						r = ustr;
					}
				}
			}
			break;
		}
	case TypeStringAnsi:
	case TypeUTF8:
		{
			char* str = buf+offset;
			int readBytes = bFromReadBuffer && info.Length() > 3 ? info[3]->Int32Value(context).FromJust() : -1;
			if(readBytes <0 || offset + readBytes <= byteLen)
			{
				wchar* wstr;
				bool nullTerminate = false;
				if(readBytes==0)
					wstr = EMPTY_STRING;
				else
				{
					if(readBytes == -1)
					{
						readBytes = strlen(str)+1;
						nullTerminate = true;
					}
					
					PiCodePage cp = type == TypeStringAnsi ? PI_CP_ACP : PI_CP_UTF8;
					wstr = pi_str_to_wstr_len(str, readBytes, cp);
				}
				int rOffset = -1;//记录下一次读取的起始位置,只有这种情形js无法计算.
				if(wstr)
				{
					r = v8::String::NewFromTwoByte(info.GetIsolate(),(uint16_t*)wstr);
					if(readBytes)
						pi_free(wstr);
					rOffset = offset + readBytes;
					*pOffset = offset + readBytes;
				}
				if(bFromReadBuffer && ibuf->IsObject())
				{
					v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(ibuf);
					obj->Set(context, OneByteString(v8::Isolate::GetCurrent(), "rOffset").ToLocalChecked(), v8::Int32::New(info.GetIsolate(),rOffset));
				}
			}
			break;
		}
	default:
		break;
	}
	return r;
}


int get_code_page_win32(PiCodePage page)
{
	int r = CP_ACP;
	switch (page)
	{
	case PI_CP_ACP:
		r = CP_ACP;
		break;
	case PI_CP_UTF8:
		r = CP_UTF8;
		break;
	case PI_CP_ISO88591:
		r = 1252;
		break;
	case PI_CP_GBK:
		r = 936;
		break;
	case PI_CP_GB2312:
		r = 20936;
		break;
	default:
		r = (int)page;
		break;
	}
	return r;
}


NAN_METHOD(VCallBinding::Register)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	Local<String> lib_name = info[0]->ToString();
	Local<String> func_name = info[1]->ToString();
	Local<Int32> ret_type = info[2]->ToInt32();
	Local<Value> params = info[3];

	int retType = ret_type->Int32Value(context).FromJust();
	if ((retType < TypeInteger) || (retType > TypeStringAnsi && retType < TypeObject) ||
		(retType == TypeBufferContent) || (retType == TypeVA)) {
		//ThrowException("vcall::register", "Register error, invalid return type!");
		//return v8::Undefined(isolate);
		info.GetReturnValue().Set(v8::Undefined(isolate));
		return;
	}

	//v8::String
	// 取动态库名和native函数名
	int resultIndex = gIndex++;
	FunItem* item = &gFunItem[resultIndex];

	std::wstring wstrtmp = V8ToWString(lib_name);
	memcpy(item->libName, wstrtmp.data(), wstrtmp.size()*sizeof(uint16_t));
	//VLOG(1) << "libname:" << item->libName << " size:" << wstrtmp.size();

	std::string strtmp = V8ToString(func_name);
	memcpy(item->funName, strtmp.data(), strtmp.size());


	// 打开动态库
	PiMod* mod = pi_mod_open(item->libName);
	if (!mod) {
		gIndex--;
		PiError* err = pi_error_get();
		char* libName = pi_wstr_to_str(item->libName, PI_CP_UTF8);
		char* errMsg = pi_wstr_to_str(err->message, PI_CP_UTF8);
		pi_log_print(LOG_WARNING, "VCallRegister pi_mod_open error, libName : %s, err msg : %s\n",
			libName, errMsg);
		pi_free(libName);
		pi_free(errMsg);

		info.GetReturnValue().Set(v8::Int32::New(isolate, ERROR_REGISTER_LOAD_LIB));
		return;
	}

	// 查找指定的函数
	item->fun = pi_mod_symbol(mod, item->funName);
	if (!item->fun) {
		gIndex--;
		PiError* err = pi_error_get();
		char* libName = pi_wstr_to_str(item->libName, PI_CP_UTF8);
		char* errMsg = pi_wstr_to_str(err->message, PI_CP_UTF8);
		pi_log_print(LOG_WARNING, "VCallRegister pi_mod_symbol error, libName : %s, funName : %s, err msg : %s\n",
			libName, item->funName, errMsg);
		pi_free(libName);
		pi_free(errMsg);

		info.GetReturnValue().Set(v8::Int32::New(isolate, ERROR_REGISTER_LOAD_LIB));
		return;
	}

	if (gLog) {
		char* libName = pi_wstr_to_str(item->libName, PI_CP_UTF8);
		//	pi_log_print(LOG_INFO, "Register OK, libName : %s, funName : %s\n",libName, item->funName);
		//VLOG(1) << "Register OK, libName : " << libName << " , funName: " << item->funName;
		pi_free(libName);
	}

	// 注册返回值和参数类型
	item->retType = (ParamType)retType;

	if (params->IsUndefined() || !params->IsArray()) {
		item->typeCollection = NULL;
		item->typeCount = 0;

		info.GetReturnValue().Set(v8::Int32::New(isolate, resultIndex));
		return;
	}

	// 设置参数类型
	Local<Array> arr = Local<Array>::Cast(params);

	int len = arr->Length();
	item->typeCount = len;
	if (len > 0) {
		item->typeCollection = new char[len];
		for (int i = 0; i < len; i++) {
			item->typeCollection[i] = arr->Get(i)->Int32Value();
		}
	}
	else {
		item->typeCollection = NULL;
	}

	info.GetReturnValue().Set(v8::Int32::New(isolate, resultIndex));
}

NAN_METHOD(VCallBinding::Invoke)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
	FunItem* pItem = NULL;

	char buf[MAX_SIZE]; // 通过反汇编发现比定义为char* buf快,因为buf[index]被编译为[ebp+offset],而char*版本的则被编译为先要取一次buf符号的值,
	char bufStr[MAX_SIZE_STR];//store contents of string only. default buffer
	char* buf2 = bufStr;
	int curSize = 0;// index of realBuf; buf[curSize] == buf + curSize == buf + realBuf - buf == realBuf
	int curSize2 = 0;
	bool newFlag;

	pi_memset(bufStr, 0, sizeof(bufStr));

	int len = info.Length();
	int index = info[0]->Int32Value(context).FromJust();
	char infoStr[300];

	if (index < 0 || index >= gIndex)
	{
		//snprintf(info, 300, "Invoke error, msg=%s, fun index=%d(%s)", GetErrorMsg(ERROR_INDEX_OUTOF_RANGE), index, GetErrorMsg(index));
		//ThrowException("invoke", info);
		return;
	}

	pItem = &gFunItem[index];
	newFlag = CheckBuffer(&buf2, info);

	/** 参数长度检查,注意可能有变参 */
	int argLen = len - 1; // 实际的参数个数(第一个参数为index)
	int regLen = pItem->typeCount; //注册的参数个数,va也算一个.
	bool bVa = regLen ? pItem->typeCollection[regLen - 1] == TypeVA : false; // 变参函数?
	if (argLen < regLen) // >=的情况都在下面进行类型匹配检查时处理
	{
		if (argLen < regLen - 1 || !bVa) // va时允许比注册的参数少一
		{
			snprintf(infoStr, 300, "Invoke error(not enough args, arg count=%d, register count=%d", argLen, regLen);
			//ThrowException("invoke", info);
			return;
		}
	}

	bool bInVa = false;//处于变参范围?
	bool bError = false;
	for (int i = 1; i < len; i++)
	{
		int type;
		v8::Local<v8::Value> Arg = info[i];
		if (bInVa)
			type = getType(Arg);
		else
		{
			type = i <= regLen ? type = pItem->typeCollection[i - 1] : 0;
			if (type == TypeVA)
			{
				bInVa = true;
				type = getType(Arg);
			}
			else if (gCheckType)
			{
				if (checkType(Arg, type) == false)
				{
					//snprintf(info, 300, "type check fails, index=%d, funName=%s, arg index(1-based)=%d, expected type=%s", index, pItem->funName, i, GetTypeStr(type));
					//ThrowException("invoke", info);
					bError = true;
					break;
				}
			}
		}
		switch (type) {
		case TypeBufferContent:
			curSize += writeBufferBuffer(buf + curSize, MAX_SIZE, 0, Arg, 0, -1); // offset=0,避免检查
			break;
		case TypeBufferPointer:
			// 如果是直接使用int作为ptr,声明为TypeInteger效率会更高.
			curSize += writeBufferBufferPointer(buf + curSize, MAX_SIZE, 0, Arg, 0);
			break;
		case TypeString:
		case TypeStringAnsi:
		case TypeUTF8:
			if (Arg->IsNull() || Arg->IsUndefined())
			{
				*(void**)(buf + curSize) = 0;
				curSize += 4;
			}
			else {
				char* pCopyDest = (buf2 + curSize2);
				*(void**)(buf + curSize) = pCopyDest;
				curSize += 4;
				int writeBytes2 = writeBufferString(buf2, 0xffffffff, (unsigned int)curSize2, Arg, type);
				curSize2 = curSize2 + writeBytes2;
			}
			break;
		case TypeInteger: // 参数提升
		case TypeUint:
		case TypeBoolean:
		case TypeByte:
		case TypeUbyte:
		case TypeShort:
		case TypeUshort:
			(*(int*)(buf + curSize)) = Arg->Int32Value(context).FromJust();
			curSize += 4;
			break;
		case TypeLong:
		case TypeUlong:
			// 8bytes alignment for 64-bits data according AAPCS.
			//if (curSize / 4 % 2 != 0)
			//	curSize += 4; 
			(*(int64*)(buf + curSize)) = (int64)Arg->NumberValue(context).FromJust();
			curSize += 8;
			break;
		case TypeFloat:
			(*(float*)(buf + curSize)) = Arg->NumberValue(context).FromJust();
			curSize += 4;
			break;
		case TypeDouble:
			// 8bytes alignment for 64-bits data according AAPCS.
			//if (curSize / 4 % 2 != 0)
			//	curSize += 4; 
			(*(double*)(buf + curSize)) = Arg->NumberValue(context).FromJust();
			curSize += 8;
			break;
		case TypeObject:
			// 8bytes alignment for 64-bits data according AAPCS.
			//if (curSize / 4 % 2 != 0)
			//	curSize += 4; 
			(*(int64*)(buf + curSize)) = Arg->IntegerValue(context).FromJust();
			curSize += 8;
			break;
		case TypeStringWithCoding:
		case TypeVA:
		case TypeVoid:
			// 参数检查能够保证不会执行到这里.
			break;
		}
	}

	if (curSize > MAX_SIZE) {
		snprintf(infoStr, 300, "Invoke error(GetParam), ERROR_ARG_SIZE_OVERFLOW, fun index=%d, funName=%s, required size=%d, max size=%d", index, pItem->funName, curSize, MAX_SIZE);
		//ThrowException("invote", info);
		printf("invote %s", info);
		bError = true;
	}

	//if (gLog) {
		//char* libName = pi_wstr_to_str(pItem->libName, PI_CP_UTF8);
		//pi_log_print(LOG_INFO, "Invoke OK, libName : %s, funName : %s\n",libName, pItem->funName);
		//pi_free(libName);
	//}

	v8::Local<v8::Value> rv = v8::Undefined(info.GetIsolate());
	if (!bError)
	{
		int64 r64;
		double dt;
		switch (pItem->retType)
		{
		case TypeInteger:
		case TypeBufferPointer:

			r64 = VCALL_INT((void*)pItem->fun, buf, curSize);
			rv = v8::Number::New(info.GetIsolate(), r64);
			break;
		case TypeByte:
			r64 = (int8)VCALL_INT((void*)pItem->fun, buf, curSize);
			rv = v8::Number::New(info.GetIsolate(), r64);
			break;
		case TypeBoolean:
			r64 = VCALL_INT((void*)pItem->fun, buf, curSize) && 0xff;
			rv = v8::Boolean::New(info.GetIsolate(), r64);
			break;
		case TypeShort:
			r64 = (int16)VCALL_INT((void*)pItem->fun, buf, curSize);
			rv = v8::Number::New(info.GetIsolate(), r64);
			break;
		case TypeLong:
			r64 = VCALL_INT64((void*)pItem->fun, buf, curSize);
			rv = v8::Number::New(info.GetIsolate(), r64);
			break;
		case TypeObject:
			r64 = VCALL_INT64((void*)pItem->fun, buf, curSize);
			rv = v8::Number::New(info.GetIsolate(), r64);
			break;
		case TypeUint:
			r64 = (uint32)VCALL_UINT((void*)pItem->fun, buf, curSize);
			rv = v8::Number::New(info.GetIsolate(), (uint64)r64);
			break;
		case TypeUbyte:
			r64 = (uint8)VCALL_UINT((void*)pItem->fun, buf, curSize);
			rv = v8::Number::New(info.GetIsolate(), (uint64)r64);
			break;
		case TypeUshort:
			r64 = (uint16)VCALL_UINT((void*)pItem->fun, buf, curSize);
			rv = v8::Number::New(info.GetIsolate(), (uint64)r64);
			break;
		case TypeUlong:
			r64 = VCALL_UINT64((void*)pItem->fun, buf, curSize);
			rv = v8::Number::New(info.GetIsolate(), (uint64)r64);
			break;
		case TypeDouble:
			dt = VCALL_DOUBLE((void*)pItem->fun, buf, curSize);
			rv = v8::Number::New(info.GetIsolate(), dt);
			break;
		case TypeFloat:
			dt = VCALL_FLOAT((void*)pItem->fun, buf, curSize);
			rv = v8::Number::New(info.GetIsolate(), dt);
			break;
		case TypeString:
		{
			// 最后需要释放
			uint strLen = 0;
			wchar* r = NULL;
			r = (wchar*)VCALL_WSTR((void*)pItem->fun, buf, curSize);
			if (r != NULL) {
				strLen = pi_wstrlen(r);
			}
			rv = v8::String::NewFromTwoByte(info.GetIsolate(), (uint16_t*)r, v8::String::kNormalString, strLen);
			pi_free(r);//special for string type
			break;
		}
		case TypeStringAnsi:
		{
			// 最后需要释放
			uint len = 0;
			char* r = NULL;
			r = VCALL_STR((void*)pItem->fun, buf, curSize);
			if (r != NULL) {
				len = pi_strlen(r);
				rv = v8::String::NewFromOneByte(info.GetIsolate(), (uint8_t*)r, v8::String::kNormalString, len);
				pi_free(r);//special for string type
			}

			break;
		}
		case TypeVoid:
			VCALL_VOID((void*)pItem->fun, buf, curSize);
			rv = v8::Undefined(info.GetIsolate());
			break;

		default:
		{
			snprintf(infoStr, 300, "unspported ret val type: %d", pItem->retType);
			//ThrowException("invoke", info);
			printf("invoke %s", info);
			rv = v8::Undefined(info.GetIsolate());
		}
		break;
		}
	}
	if (newFlag)
	{
		delete buf2;
	}
	//printf("Invoke finish and return...\n");
	info.GetReturnValue().Set(rv);
}

NAN_METHOD(VCallBinding::ParseCmdList)
{
	Isolate* isolate = info.GetIsolate();
	Local<v8::Int32> pibytes = info[0]->ToInt32();

	PiBytes *bb = (PiBytes*)(pibytes->Int32Value());
	int size = pi_bytes_size(bb);
	while (size > (int)pi_bytes_rindex(bb, -1)) {

		int paramSize = 0;
		PiFunc func = NULL;
		char *param = NULL;
		char *bufValue = NULL;
		pi_bytes_read_pointer(bb, (void **)&func);
		pi_bytes_read_pointer(bb, (void **)&bufValue);
		pi_bytes_read_int(bb, (sint *)&paramSize);
		pi_bytes_read_data(bb, (void **)&param, paramSize);

		VCALL_VOID((void*)func, param, paramSize);

		if (bufValue != NULL) {
			pi_free(bufValue);
		}
	}

	info.GetReturnValue().Set(v8::Undefined(isolate));
}

NAN_METHOD(VCallBinding::FillCmdList)
{
	v8::HandleScope handle_scope(info.GetIsolate());

	{
		/* 字符串参数，buffer参数的缓冲区 */
		char *bufValue;
		int curBufSize = 0;
		int paramSizeIndex = 0;
		int paramSize = 0;

		int len = info.Length();
		int index = info[0]->Int32Value();
		PiBytes *bb = (PiBytes*)(info[1]->Int32Value());
		//char buf[300];

		if (index < 0 || index >= gIndex)
		{
			//sprintf_s(buf, 300, "Invoke error, msg=%s, fun index=%d(%s)", GetErrorMsg(ERROR_INDEX_OUTOF_RANGE), index, vcall::GetErrorMsg(index));
			//ThrowException("FillCmdList", buf);
			info.GetReturnValue().Set(v8::Undefined(info.GetIsolate()));
			return;
		}

		FunItem* pItem = &gFunItem[index];
		bufValue = GetParamValueBuffer(info);


// 		if (gRecordInvokeFun)
// 			TlsSetValue(tlsFunIndex, (void*)index);

		/** 参数长度检查 */
		int argLen = len - 2;		   // 实际的参数个数(第一个参数为index，第二个为pibytes)
		int regLen = pItem->typeCount; // 注册的参数个数

									   // VA则报错
		if (regLen > 0 && pItem->typeCollection[regLen - 1] == TypeVA)
		{
			//ThrowException("FillCmdList", "InvokeFill error(can't use VA)");
			info.GetReturnValue().Set(v8::Undefined(info.GetIsolate()));
			return;
		}

		if (argLen < regLen) // >=的情况都在下面进行类型匹配检查时处理
		{
			if (argLen < regLen - 1)
			{
				//sprintf_s(buf, 300, "Invoke error(not enough args, arg count=%d, register count=%d", argLen, regLen);
 				//ThrowException("FillCmdList", buf);
				info.GetReturnValue().Set(v8::Undefined(info.GetIsolate()));
				return;
			}
		}

		// 函数指针 
		pi_bytes_write_pointer(bb, pItem->fun);

		// 拷贝的参数值的指针
		pi_bytes_write_pointer(bb, bufValue);

		// 参数字节数
		pi_bytes_write_int(bb, 0);
		paramSizeIndex = pi_bytes_windex(bb, -1) - 4;

		// 参数数据
		paramSize = 0;

		bool bError = false;
		for (int i = 2; i < len; i++)
		{
			Local<Value> arg = info[i];
			int type = i <= regLen + 1 ? type = pItem->typeCollection[i - 2] : 0;
			if (gCheckType)
			{
				if (checkType(arg, type) == false)
				{
					//sprintf_s(buf, 300, "type check fails, index=%d, funName=%s, arg index(1-based)=%d, expected type=%s", index, pItem->funName, i, vcall::GetTypeStr(type));
					//ThrowException("FillCmdList", buf);
					bError = true;
					break;
				}
			}

			//int r = 1;
			switch (type) {
			case TypeBufferContent:
			case TypeBufferPointer:
			{
				int bufSize = 0;
				char *pCopyDest = bufValue + curBufSize;
				if (arg->IsNull() || arg->IsUndefined()) {
					pCopyDest = NULL;
				}
				else if (arg->IsNumber()) {
					pCopyDest = (char *)arg->Int32Value();
				}
				else {
					char *p = (char*)GetArrayBuffer(arg, (unsigned int *)&bufSize);
					if (p == NULL || bufSize == 0) {
						pCopyDest = NULL;
					}
					else {
						pi_memcpy(pCopyDest, p, bufSize);
					}
				}
				curBufSize += bufSize;
				pi_bytes_write_pointer(bb, pCopyDest);
				paramSize += sizeof(char *);
				break;
			}
			case TypeString:
			case TypeStringAnsi:
			case TypeUTF8:
			{
				int bufSize = 0;
				char *pCopyDest = bufValue + curBufSize;
				if (arg->IsNull() || arg->IsUndefined()) {
					pCopyDest = NULL;
				}
				else {
					bufSize = writeBufferString(bufValue, 0xffffffff, (unsigned int)curBufSize, arg, type);
				}
				curBufSize += bufSize;
				pi_bytes_write_pointer(bb, pCopyDest);
				paramSize += sizeof(char *);
				break;
			}
			case TypeInteger: // 参数提升
			case TypeUint:
			case TypeBoolean:
			case TypeByte:
			case TypeUbyte:
			case TypeShort:
			case TypeUshort:
				pi_bytes_write_int(bb, arg->Int32Value());
				paramSize += sizeof(int32);
				break;
			case TypeLong:
			case TypeUlong:
				pi_bytes_write_int64(bb, (int64)arg->NumberValue());
				paramSize += sizeof(int64);
				break;
			case TypeFloat:
				pi_bytes_write_float(bb, (float)arg->NumberValue());
				paramSize += sizeof(float);
				break;
			case TypeDouble:
				pi_bytes_write_double(bb, arg->NumberValue());
				paramSize += sizeof(double);
				break;
			case TypeObject:
				pi_bytes_write_int64(bb, arg->IntegerValue());
				paramSize += 8;
				break;
			case TypeStringWithCoding:
			case TypeVA:
			case TypeVoid:
				// 参数检查能够保证不会执行到这里.
				break;
			}
		}

		// 将参数字节数写入对应的位置
		char *arr = (char *)pi_bytes_array(bb, 0);
		int *pParamSize = (int *)(arr + paramSizeIndex);
		*pParamSize = paramSize;
	}


	info.GetReturnValue().Set(v8::Undefined(info.GetIsolate()));
}

NAN_METHOD(VCallBinding::WriteBuffer)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
	int len = info.Length();
	v8::Local<v8::Value> argument = info[0];
	v8::Local<v8::Value> obuf = info[1];
	int type = info[2]->Int32Value(context).FromJust();

	unsigned int byteLen;
	char* buf;
	if (!tryGetBuffer(obuf, &buf, &byteLen)) {
		//ThrowException("WriteBuffer","WriteBuffer error, invalid buffer");
		info.GetReturnValue().Set(v8::Undefined(info.GetIsolate()));
		return;
	}

	unsigned int offset = info[3]->Uint32Value(context).FromJust();
	int writeBytes = 0;
	if (type == TypeBufferContent) {
		int srcOffset = info[4]->Uint32Value(context).FromJust();
		int srcLen = info[5]->Uint32Value(context).FromJust();
		writeBytes = writeBufferBuffer(buf, byteLen, offset, argument, srcOffset, srcLen);
	}
	else if (type == TypeBufferPointer) {
		int srcOffset = len > 4 ? info[4]->Int32Value(context).FromJust() : 0;
		writeBytes = writeBufferBufferPointer(buf, byteLen, offset, argument, srcOffset);
	}
	else if (type == TypeString || type == TypeStringAnsi || type == TypeUTF8) {
		writeBytes = writeBufferString(buf, byteLen, offset, argument, type);
	}
	else {
		writeBytes = writeBufferNumber(buf, byteLen, offset, argument, type);
	}
	info.GetReturnValue().Set(writeBytes);
}

NAN_METHOD(VCallBinding::ReadBuffer)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

	v8::Local<v8::Value> ibuf = info[0];
	int type = info[1]->Int32Value(context).FromJust();
	unsigned int offset = info[2]->Uint32Value(context).FromJust();
	info.GetReturnValue().Set(readBufferCore(true, info, ibuf, &offset, type));
}

NAN_METHOD(VCallBinding::GetBufferAddress)
{
	v8::Isolate* isolate = v8::Isolate::GetCurrent();

	Local<Value> buf = info[0];
	if (buf->IsArrayBuffer())
	{
		info.GetReturnValue().Set(v8::Undefined(isolate));
	}
	else if (buf->IsArrayBufferView()) 
	{
		Local<ArrayBufferView> bv = Local<ArrayBufferView>::Cast(buf);

		void* ret = bv->Buffer()->GetContents().Data();

		info.GetReturnValue().Set(Integer::New(isolate, reinterpret_cast<int32_t>(ret)));
	}
	else
	{
		info.GetReturnValue().Set(v8::Undefined(isolate));
	}
}

NAN_METHOD(VCallBinding::ReadJson)
{
	v8::HandleScope handle_scope(info.GetIsolate());

	info.GetReturnValue().SetUndefined();
	if (info.Length()> 1)
	{
		char* buf = (char*)info[0]->Int32Value();
		uint len = info[1]->Uint32Value();
		if (buf && len)
		{
			v8::Local<v8::Value> r = v8::JSON::Parse(v8::String::NewFromUtf8(info.GetIsolate(), buf, v8::String::kNormalString, len));
			info.GetReturnValue().Set(r);
		}
	}
}

NAN_METHOD(VCallBinding::GetHandle)
{
	//Isolate* isolate = info.GetIsolate();
	Local<Value> v = info[0];

	info.GetReturnValue().Set(v->Int32Value());
}

NAN_METHOD(VCallBinding::HandleToVar)
{
	info.GetReturnValue().Set(v8::Undefined(info.GetIsolate()));
}

NAN_METHOD(VCallBinding::UpdateObjFromArrayBuffer)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Local<v8::Array> arrbuf = v8::Local<v8::Array>::Cast(info[2]);
	info.GetReturnValue().Set(UpdateObj(info, info[0], arrbuf, info[1], info[3]));
}

NAN_METHOD(VCallBinding::CreateObjFromArrayBuffer)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Local<v8::Object> obj = v8::Object::New(info.GetIsolate());
	info.GetReturnValue().Set(UpdateObj(info, obj, v8::Local<v8::Array>::Cast(info[1]), info[0], info[3], true));
}

NAN_METHOD(VCallBinding::openEXE)
{
	Isolate* isolate = info.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HandleScope handle_scope(isolate);

	Local<Value> strEXE(info[0]);
	Local<Value> strCmd(info[1]);

	SettingsService *tpSettingService = (SettingsService *)PiGameServiceManager::GetInstance()->GetServiceByName(L"Settings");

	info.GetReturnValue().Set(GameTools::openEXE(tpSettingService->getWordDir(), V8ToWString(strEXE), V8ToWString(strCmd)));
}

NAN_METHOD(VCallBinding::UpdateArrayBufferFromObject){
	v8::HandleScope handle_scope(info.GetIsolate());
	info.GetReturnValue().Set(UpdateObj(info, info[0], v8::Local<v8::Array>::Cast(info[2]), info[1], info[3], false));
}

NAN_METHOD(VCallBinding::VCallGC)
{
	v8::HandleScope handle_scope(info.GetIsolate());
	v8::Isolate* isolate = info.GetIsolate();

	enum { ALL_GC, FULL_GC, IDLE_GC } gc_type;
	gc_type = FULL_GC;
	int time_limit = 0;

	// pi.gc(milliSeconds) || pi.gc(-1)
	if (info.Length() > 0 && info[0]->IsNumber())
	{
		time_limit = info[0]->IntegerValue();
		if (time_limit < 0)
		{
			gc_type = ALL_GC;
		}
		else if (time_limit == 0)
		{
			gc_type = FULL_GC;
		}
		else
		{
			gc_type = IDLE_GC;
		}
	}

	if (gc_type == ALL_GC)
	{
		isolate->LowMemoryNotification();
	}
	else if (gc_type == FULL_GC)
	{
		// same as: isolate->heap()->CollectAllGarbage(v8::Heap::kNoGCFlags);
		//isolate->RequestGarbageCollectionForTesting(v8::Isolate::kFullGarbageCollection);
	}
	else
	{
		isolate->IdleNotification(time_limit);
	}

}


NAN_METHOD(VCallBinding::ExecuteStr)
{
	Isolate* isolate = info.GetIsolate();
	HandleScope handle_scope(isolate);
	Local<Value> file(info[0]);

	std::wstring str = V8ToWString(file);
	const wchar_t* content = str.c_str();

	v8::Local<v8::Value> result = VCallBinding::ExecuteString(
									isolate, 
									v8::String::NewFromTwoByte(isolate, (uint16_t*)content), 
									v8::String::NewFromUtf8(isolate, "unnamed"));

	info.GetReturnValue().Set(result);
}

NAN_METHOD(VCallBinding::LoadScriptFromStr)
{
	Isolate* isolate = info.GetIsolate();
	HandleScope handle_scope(isolate);
	Local<Value> name(info[0]);
	Local<Value> file(info[1]);

	std::wstring str = V8ToWString(file);
	const wchar_t* content = str.c_str();

	v8::Local<v8::Value> result = VCallBinding::ExecuteString(
		isolate, 
		v8::String::NewFromTwoByte(isolate, (uint16_t*)content), 
		name);

	info.GetReturnValue().Set(result);
}


NAN_METHOD(VCallBinding::ClearConsole)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD coordScreen = { 0, 0 };    // home for the cursor 
	DWORD cCharsWritten;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD dwConSize;

	// Get the number of character cells in the current buffer. 

	if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
		return;
	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

	// Fill the entire screen with blanks.

	if (!FillConsoleOutputCharacter(hConsole, (TCHAR) ' ',
		dwConSize, coordScreen, &cCharsWritten))
		return;

	// Get the current text attribute.

	if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
		return;

	// Set the buffer's attributes accordingly.

	if (!FillConsoleOutputAttribute(hConsole, csbi.wAttributes,
		dwConSize, coordScreen, &cCharsWritten))
		return;

	// Put the cursor at its home coordinates.

	SetConsoleCursorPosition(hConsole, coordScreen);
}

NAN_METHOD(VCallBinding::PrintParamType)
{
	PrintType(info[0]);

	info.GetReturnValue().Set(v8::Undefined(info.GetIsolate()));
}

NAN_METHOD(VCallBinding::CreateObjectWithName)
{
	v8::HandleScope handle_scope(info.GetIsolate());

	v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(info.GetIsolate());
	t->SetClassName(info[0]->ToString());

	info.GetReturnValue().Set(t->GetFunction()->NewInstance());
}

NAN_METHOD(VCallBinding::GetHiddenValue)
{
	// todo
}

NAN_METHOD(VCallBinding::SetHiddenValue)
{
	// todo
}

NAN_METHOD(VCallBinding::GetObjectHash)
{
	Local<Object> object = info[0]->ToObject();
	
	info.GetReturnValue().Set(object->GetIdentityHash());
}

NAN_METHOD(VCallBinding::SetDestructor)
{
	Isolate* isolate = info.GetIsolate();
	Local<Object> object = info[0]->ToObject(isolate);
	Local<Function> callbackFunction = Local<Function>::Cast(info[1]);
	auto callback = Persistent<Function, CopyablePersistentTraits<Function> >(isolate, callbackFunction);
	RemoteObjectFreer::BindTo(isolate, object, callback);
}

NAN_METHOD(VCallBinding::TakeHeapSnapshot)
{
	info.GetIsolate()->GetHeapProfiler()->TakeHeapSnapshot();
}

NAN_METHOD(VCallBinding::LinkWeakRef)
{
	// TODO
}

NAN_METHOD(VCallBinding::GetWeakRef)
{
	// TODO
}

NAN_METHOD(VCallBinding::CreateWeakRef)
{
	// TODO
}

NAN_METHOD(VCallBinding::SetVfsMountPath)
{
	Local<String> v8_map_path = info[0]->ToString();
	Local<String> v8_mod_name = info[1]->ToString();
	Local<Value> v8_mode_type = info[2];
	Local<String> v8_real_path = info[3]->ToString();


	std::wstring map_path = V8ToWString(v8_map_path);
	std::wstring mod_name = V8ToWString(v8_mod_name);
	std::wstring real_path = V8ToWString(v8_real_path);

	void* mode = (void*)(v8_mode_type->Uint32Value());

	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	SettingsService* setttingService = (SettingsService *)tpManager->GetServiceByName(L"Settings");
	std::wstring path = setttingService->getWordDir().c_str();

	if (path.size() > 0 && path[path.size() - 1] != L'\\')
	{
		path = path.append(L"\\");
	}

	if (real_path != L".") {
		path = path.append(real_path);
	}

	int ret = pi_vfs_mount(map_path.c_str(), mod_name.c_str(), mode, path.c_str());

	info.GetReturnValue().Set(v8::Int32::New(info.GetIsolate(), ret));
}

NAN_METHOD(VCallBinding::GetCommandLineString)
{
	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	SettingsService* setttingService = (SettingsService *)tpManager->GetServiceByName(L"Settings");
	
	std::wstring cmdstr = setttingService->getCommandLine();

	info.GetReturnValue().Set(
		v8::String::NewFromTwoByte(info.GetIsolate(), (uint16_t*)(cmdstr.c_str()), v8::String::kNormalString,
		cmdstr.length()));
}

NAN_METHOD(VCallBinding::HttpRequest)
{
    Isolate* isolate = info.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    HandleScope handle_scope(isolate);

    Local<Value> file(info[0]);
    std::wstring str = V8ToWString(file);

    Local<Function> callbackFunction = Local<Function>::Cast(info[1]);
    auto callback = Persistent<Function, CopyablePersistentTraits<Function> >(isolate, callbackFunction);

    HttpRequestTask* task = new HttpRequestTask();
    task->SetParam(str, callback);
    GameTools::AsyncInvoke(reinterpret_cast<pi::Task*>(task));
}

NAN_METHOD(VCallBinding::GetWinHandle)
{

	PiGameServiceManager *tpManager = PiGameServiceManager::GetInstance();
	WindowsService* wndService = (WindowsService *)tpManager->GetServiceByName(L"WindowsService");

	Isolate* isolate = info.GetIsolate();
	info.GetReturnValue().Set(
		Int32::New(isolate, reinterpret_cast<int32>(wndService->GetWindowHandle())));
}

NAN_METHOD(VCallBinding::Log)
{
	LogType type = (LogType)info[0]->Uint32Value();
	Local<String> strArg = info[1]->ToString();
	std::wstring str = V8ToWString(strArg);
	char* logStr = pi_wstr_to_str_len(str.c_str(), str.size(), PI_CP_ACP);
	pi_log_print(type, "[%d] %s", GetCurrentProcessId(), logStr);
	if (logStr != NULL) 
	{
		pi_free(logStr);
	}
}

NAN_METHOD(VCallBinding::Load)
{
	Isolate* isolate = info.GetIsolate();
	HandleScope handle_scope(isolate);
	Local<Value> file(info[0]);

	std::wstring str = V8ToWString(file);
	wchar_t* content = VfsReadAll(str.c_str());

	v8::Local<v8::Value> result = VCallBinding::ExecuteString(isolate, v8::String::NewFromTwoByte(isolate, (uint16_t*)content), info[0]);
	pi_free(content);

	info.GetReturnValue().Set(result);
}


Local<Value> VCallBinding::ExecuteString(Isolate* isolate,
	Local<String> source,
	Local<Value> name) {
	EscapableHandleScope scope(isolate);
	v8::ScriptOrigin origin(name);

	TryCatch try_catch(isolate);
	try_catch.SetVerbose(false);

	Local<v8::Script> script;
	if (!v8::Script::Compile(isolate->GetCurrentContext(), source, &origin).ToLocal(&script)) {
		Backtrace::GetInstance().ReportException(isolate, &try_catch);
	}

	Local<v8::Value> result;
	if (!script->Run(isolate->GetCurrentContext()).ToLocal(&result)) {
		Backtrace::GetInstance().ReportException(isolate, &try_catch);
	}

	return scope.Escape(result);
}

wchar* VCallBinding::VfsReadAll(const wchar* path) {
	int read;
	void* file;
	file = pi_vfs_file_open(path, FILE_OPEN_READ);
	if (file) {
		int64 len;
		if (pi_vfs_file_size(file, &len)) {
			char* buf = (char*)pi_malloc((uint)len + 1);
			read = pi_vfs_file_read(file, 0, 0, buf, (uint)len);
			if (read) {
				wchar* content;
				buf[len] = 0;
				content = pi_str_to_wstr(buf, PI_CP_UTF8);
				pi_free(buf);
				pi_vfs_file_close(file);
				return content;
			}
		}
		pi_vfs_file_close(file);
	}
	pi_error_log(path);
	return NULL;
}
