#ifndef _PI_VCALL_H
#define _PI_VCALL_H

#include <pi_lib.h>
#include "v8.h"

#include "nan/nan.h"

// 最多可注册函数的个数
#define MAX_FUN_NUM  10240
#define MAX_NAME_LEN 260
/**
 * 错误消息文字的分配大小
 */
#define ERROR_MESSAGE_MALLOC_SIZE 1024
/**
 * 文件名的分配大小
 */
#define ERROR_FILE_NAME_SIZE 260

#define MATE_INTEGER_NEW_UNSIGNED(isolate, data) \
  v8::Integer::NewFromUnsigned(isolate, data)
#define MATE_INTEGER_NEW(isolate, data) v8::Integer::New(isolate, data)


v8::Local<v8::Value> ConverterToV8_int32_t(v8::Isolate* isolate,int32_t val);
v8::Local<v8::Value> ConverterToV8_uint32_t(v8::Isolate* isolate,uint32_t val);


int get_code_page_win32(PiCodePage page);
char*	pi_wstr_to_str_len(const wchar *wstr, uint len, PiCodePage page);
wchar*	pi_str_to_wstr_len(const char *str, uint len, PiCodePage page);

typedef enum
{
    TypeInteger = 2,
    TypeUint,
    TypeBoolean,
    TypeByte,
    TypeUbyte,
    TypeShort,
    TypeUshort,
    TypeLong,
    TypeUlong,
    TypeFloat,
    TypeDouble,
    TypeString,
    TypeStringAnsi,        // 本地编码
    TypeUTF8,
    TypeStringWithCoding,  // 高16位制定code page,暂不支持
    TypeObject,
    TypeBufferPointer,
    TypeBufferContent,
    TypeVA,                // 变参
    TypeVoid,
} ParamType;

// 函数条目结构
typedef struct
{
    PiFunc          fun;                    /* 函数指针 */
    wchar           libName[MAX_NAME_LEN];  /* 库名字 */
    char            funName[MAX_NAME_LEN];  /* 函数名字 */
    ParamType       retType;                /* 返回值类型 */
    char            strType[8];             /* string参数类型的具体类型,默认为unicode,最多支持8个string参数 */
    char*           typeCollection;         /* 函数参数类型容器 */
    unsigned char   typeCount;              /* 函数参数类型容器的容量 */
} FunItem;

extern FunItem gFunItem[MAX_FUN_NUM];
extern int gIndex; // js中0表示未初始化,因此g_funItem[0]不使用

// GetParam [1, 50]
#define ERROR_ARG_SIZE_OVERFLOW		-1
#define ERROR_UNKNOW_ARG_TYPE		-3
#define ERROR_ARG_UNDEFINED			-4
#define ERROR_ARG_INVALID_BUFFER	-5

// Register[51, 100]
#define ERROR_REGISTER_INVALID_ARG -51		//参数错误
#define ERROR_REGISTER_LOAD_LIB    -52		//
#define ERROR_REGISTER_LOAD_FUN    -53		//

// Invoke[101, 150]
#define ERROR_INDEX_OUTOF_RANGE -101
 
// const char* GetErrorMsg(int id);
// const char* GetTypeStr(int id);

int getType(v8::Local<v8::Value> val);
bool checkType(v8::Local<v8::Value> val, int expectedType);


v8::Local<v8::Value> readBufferCore(bool bFromReadBuffer, Nan::NAN_METHOD_ARGS_TYPE  args, v8::Local<v8::Value> ibuf, unsigned int* pOffset, int type);
inline int writeBufferString(char* buf, unsigned int byteLen, unsigned int offset, v8::Local<v8::Value> Val, int type);


class VCallBinding 
{
public:
	// log
	static NAN_METHOD(Log);
	// vcall
	static NAN_METHOD(Register);
	static NAN_METHOD(Invoke);

	static NAN_METHOD(ParseCmdList);
	static NAN_METHOD(FillCmdList);

	static NAN_METHOD(WriteBuffer);
	static NAN_METHOD(ReadBuffer);
	static NAN_METHOD(GetBufferAddress);

	static NAN_METHOD(ReadJson);

	static NAN_METHOD(GetHandle);
	static NAN_METHOD(HandleToVar);

	static NAN_METHOD(UpdateObjFromArrayBuffer);
	static NAN_METHOD(CreateObjFromArrayBuffer);
	static NAN_METHOD(UpdateArrayBufferFromObject);

	static NAN_METHOD(VCallGC);
	static NAN_METHOD(ExecuteStr);
	static NAN_METHOD(LoadScriptFromStr);
	static NAN_METHOD(openEXE);

	static void VCallGC(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void ExecuteStr(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void LoadScriptFromStr(const v8::FunctionCallbackInfo<v8::Value>& args);

	// Platform tools
	static NAN_METHOD(Load);
	static NAN_METHOD(ClearConsole);
	static NAN_METHOD(PrintParamType);

	// v8 tools
	static NAN_METHOD(CreateObjectWithName);
	static NAN_METHOD(GetHiddenValue);
	static NAN_METHOD(SetHiddenValue);
	static NAN_METHOD(GetObjectHash);
	static NAN_METHOD(SetDestructor);
	static NAN_METHOD(TakeHeapSnapshot);
	static NAN_METHOD(LinkWeakRef);
	static NAN_METHOD(GetWeakRef);
	static NAN_METHOD(CreateWeakRef);

	// vfs: pi.vfsMount
	static NAN_METHOD(SetVfsMountPath);

	// commandline: pi.getCommandline()
	static NAN_METHOD(GetCommandLineString);

	static NAN_METHOD(HttpRequest);
	static NAN_METHOD(GetWinHandle);

	static v8::Local<v8::Value> ExecuteString(v8::Isolate* isolate,
		v8::Local<v8::String> source,
		v8::Local<v8::Value> name);

	static wchar* VfsReadAll(const wchar* path);

};

#endif // HZ_VCALL_H
