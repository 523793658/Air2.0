/**
 * 该文件主要处理DDS格式
 */
#include <image.h>

/**
 * NV的DDS库，用于压缩DDS文件
 */
#include <dxtlib/dxtlib.h>
#include <dds/nvErrorCodes.h>
 
static int mul_8_bit(int a, int b)
{
	int t = a * b + 128;
	return((t + (t >> 8)) >> 8);
}

static int blerp(int a, int b, int x)
{
	return(a + mul_8_bit(b - a, x));
}

static void lerp_rgb(byte *dst, byte *a, byte *b, int f)
{
	dst[0] = (byte)(blerp(a[0], b[0], f));
	dst[1] = (byte)(blerp(a[1], b[1], f));
	dst[2] = (byte)(blerp(a[2], b[2], f));
}

static uint16 get_L16(byte *buf)
{
	return ((uint16)buf[0]) |  ((uint16)buf[1] << 8);
}

static int64 get_L64(byte *buf)
{
	return 
	((int64)buf[0])       |
	((int64)buf[1] <<  8) |
	((int64)buf[2] << 16) |
	((int64)buf[3] << 24) |
	((int64)buf[4] << 32) |
	((int64)buf[5] << 40) |
	((int64)buf[6] << 48) |
	((int64)buf[7] << 56);
}

PI_BEGIN_DECLS

/* pack BGR8 to RGB565 */
uint16 pack_rgb_565(byte *c)
{
	return(((c[2] >> 3) << 11) | ((c[1] >> 2) << 5) | (c[0] >> 3));
}

/* unpack RGB565 to BGR */
void unpack_rgb_565(byte  *dst, uint16 v)
{
	int r = (v >> 11) & 0x1f;
	int g = (v >>  5) & 0x3f;
	int b = (v      ) & 0x1f;

	dst[0] = (byte)((b << 3) | (b >> 2));
	dst[1] = (byte)((g << 2) | (g >> 4));
	dst[2] = (byte)((r << 3) | (r >> 2));
}

void dxt_decode_rgb(byte *dst, byte *src, EImageType format)
{
	int i, x, y;
	byte colors[4][3];
	uint indexes, idx;
	uint16 c0 = get_L16(&src[0]);
	uint16 c1 = get_L16(&src[2]);

	unpack_rgb_565(colors[0], c0);
	unpack_rgb_565(colors[1], c1);
	if((c0 > c1) || (EImT_DXT5 == format))
	{
		lerp_rgb(colors[2], colors[0], colors[1], 0x55);
		lerp_rgb(colors[3], colors[0], colors[1], 0xaa);
	}
	else
	{
		for(i = 0; i < 3; ++i)
		{
			colors[2][i] = (colors[0][i] + colors[1][i] + 1) >> 1;
			colors[3][i] = 255;
		}
	}

	src += 4;
	for(y = 0; y < 4; ++y)
	{
		indexes = src[y];
		for(x = 0; x < 4; ++x)
		{
			idx = indexes & 0x03;
			dst[2] = colors[idx][2];
			dst[1] = colors[idx][1];
			dst[0] = colors[idx][0];
			if(EImT_DXT1 == format)
				dst[3] = ((c0<=c1) && (idx==3)) ? 0 : 255;
			indexes >>= 2;
			dst += 4;
		}
	}
}

void dxt3_decode_alpha(byte *dst, byte *src)
{
	int x, y;
	uint bits;

	for(y = 0; y < 4; ++y)
	{
		bits = get_L16(&src[2 * y]);
		for(x = 0; x < 4; ++x)
		{
			dst[3] = (bits & 0x0f)*17;
			bits >>= 4;
			dst += 4;
		}
	}
}

void dxt5_decode_alpha(byte *dst, byte *src)
{
	int x, y, code;
	byte a0 = src[0];
	byte a1 = src[1];
	int64 bits = get_L64(src) >> 16;
	for(y = 0; y < 4; ++y)
	{
		for(x = 0; x < 4; ++x)
		{
			code = ((uint)bits) & 0x07;
			if(code == 0)
				dst[3] = a0;
			else if(code == 1)
				dst[3] = a1;
			else if(a0 > a1)
				dst[3] = (byte)(((8 - code) * a0 + (code - 1) * a1) / 7);
			else if(code >= 6)
				dst[3] = (code == 6) ? 0 : 255;
			else
				dst[3] = (byte)(((6 - code) * a0 + (code - 1) * a1) / 5);
			bits >>= 3;
			dst += 4;
		}
	}
}

struct DXTCallBakData
{
	byte *ddsData;
	uint num;	//每次调用中回调函数的次数
	uint len;	//已经处理的大小
	uint maxLen; //数据总大小
};

NV_ERROR_CODE dxt_write_callback(const void *buffer, size_t count, const MIPMapData *, void * userData)
{
	DXTCallBakData* pCallbackData = static_cast<DXTCallBakData*>(userData);

	//前两次调用属于文件头部分，丢弃它
	if(pCallbackData->num > 1)
	{
		//因为一块只有16字节
		assert(count == 16);

		memcpy_s(pCallbackData->ddsData, count, buffer, count);
		pCallbackData->len += count;
	}
	pCallbackData->num++;

	return pCallbackData->len <= pCallbackData->maxLen ? NV_OK : NV_READ_FAILED;
}

void dxt_compress_block(byte *dst, size_t size, byte *src, EImageType format)
{
	//用nv提供的压缩库
	nvCompressionOptions options;
	//质量用最高级别
	options.quality = kQualityHighest;

	//格式根据文件格式来
	switch(format)
	{
	case EImT_DXT1:
		options.textureFormat = kDXT1;
		break;
	case EImT_DXT3:
		options.textureFormat = kDXT3;
		break;
	case EImT_DXT5:
		options.textureFormat = kDXT5;
		break;
	}
	//不需要Mip链
	options.DoNotGenerateMIPMaps();

	//自定义数据，供下面的压缩回调
	DXTCallBakData dXTCallBakData;
	dXTCallBakData.len = 0;
	dXTCallBakData.maxLen = size;

	options.user_data = &dXTCallBakData;	//用户数据

	dXTCallBakData.num = 0;
	dXTCallBakData.ddsData = dst;
	nvDDS::nvDXTcompress(src, 4, 4, 16, nvBGRA, &options,dxt_write_callback);
}

PI_END_DECLS