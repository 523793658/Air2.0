#include "pi_lzf.h"
#include "lzf.h"
# include <errno.h>

#define MAX_BLOCKSIZE 65535
#define MAX_HDR_SIZE 7
#define MIN_HDR_SIZE 5


#if WIN32
# define GET_ERRNO(var) _get_errno(var)
#else
# define GET_ERRNO(var) *var = 1 
#endif


typedef struct lzf_stream
{
	uint8 *buf_in;                     /*���뻺����*/
	uint avail_in;                     /*���뻺������ʣ����ó���*/
	uint avail_usage;                  /*���뻺������ʹ�õĳ���*/

	uint8 *buf_out;                    /*���������*/
	uint pending;                      /*δ����Ļ������ݵĳ���*/
	uint flushed;                      /*��������������ĳ���*/
} LZFStream;

// lzf��Ĵ����봦��
static void _lzf_error_set()
{
	int err;
	wchar *msg;
	GET_ERRNO(&err);
	if(E2BIG == err)
	{
		msg = L"the output buffer is not large enough to hold the decompressed data";
	}
	else if(EINVAL == err)
	{
		msg = L"an error in the compressed data is detected";
	}
	else
	{
		msg = L"unknown error";
	}
	pi_error_set(ERROR_TYPE_INTERNAL, err, msg, __FILE__, __LINE__);
	pi_free(msg);
}

uint32 pi_lzf_compress(const void * source, uint32 sourceLen, void * dest, uint32 destLen)
{
	uint32 err = lzf_compress(source, sourceLen, dest, destLen);
	if(err == 0)
	{
		pi_error_set(ERROR_TYPE_INTERNAL, 0, L"the output buffer is not large enough", __FILE__, __LINE__);
	}
	return err;
}

uint32 pi_lzf_uncompress(const void * source, uint32 sourceLen, void * dest, uint32 destLen)
{
	uint32 err = lzf_decompress(source, sourceLen, dest, destLen);
	if(err == 0)
	{
		_lzf_error_set();
	}
	return err;
}

void * pi_lzf_compress_init()
{
	LZFStream *strm = pi_new0(LZFStream, 1);

	strm->avail_in = MAX_BLOCKSIZE;
	strm->avail_usage = 0;
	strm->buf_in = pi_malloc(MAX_BLOCKSIZE);

	strm->flushed = 0;
	strm->pending = 0;
	strm->buf_out = pi_malloc(MAX_BLOCKSIZE + MAX_HDR_SIZE);
	return strm;
}

// ����������������������next_out
static void out_pending(CompressContext * context)
{
	LZFStream* stream = (LZFStream*)context->strm;
	uint len = MIN(stream->pending, context->avail_out);
	if (len == 0)
		return;
	pi_memcpy(context->next_out, &stream->buf_out[stream->flushed], len);
	context->next_out = &context->next_out[len];
	context->avail_out -= len;
	context->total_out += len;
	stream->pending -= len;
	stream->flushed += len;
}
// ��ȡ���ݵ�������������
static void in_pending(CompressContext * context)
{
	LZFStream * stream = (LZFStream*)context->strm;
	uint len = MIN(context->avail_in, stream->avail_in);
	if(len == 0)
		return;
	pi_memcpy(&stream->buf_in[stream->avail_usage], context->next_in, len);
	context->next_in = &context->next_in[len];
	context->total_in += len;
	context->avail_in -= len;
	stream->avail_in -= len;
	stream->avail_usage += len;
}
//����ͷ��ѹ������
static void lzf_compress_head(CompressContext * context)
{
	LZFStream *strm = (LZFStream*)context->strm;
	uint srcLen = strm->avail_usage;
	uint dstLen = lzf_compress(strm->buf_in, srcLen, &strm->buf_out[MAX_HDR_SIZE], MAX_BLOCKSIZE);
	if(dstLen > 0 && dstLen < srcLen)
	{
		/*ѹ���ɹ�*/
		strm->buf_out[0] = 'Z';
		strm->buf_out[1] = 'V';
		strm->buf_out[2] = 1;
		strm->buf_out[3] = (uint8)(dstLen >> 8);
		strm->buf_out[4] = dstLen & 0xff;
		strm->buf_out[5] = (uint8)(srcLen >> 8);
		strm->buf_out[6] = srcLen & 0xff;
		strm->pending = MAX_HDR_SIZE + dstLen;
	}
	else
	{
		/*ѹ��ʧ�ܣ�����ѹ���󳤶ȴ���δѹ���ĳ��ȣ���������¾Ͳ�ѹ������head�б���*/
		strm->buf_out[0] = 'Z';
		strm->buf_out[1] = 'V';
		strm->buf_out[2] = 0;
		strm->buf_out[3] = (uint8)(srcLen >> 8);
		strm->buf_out[4] = srcLen & 0xff;
		pi_memcpy(&strm->buf_out[MIN_HDR_SIZE], strm->buf_in, srcLen);
		strm->pending = MIN_HDR_SIZE + srcLen;
	}
	strm->flushed = 0;
	strm->avail_in = MAX_BLOCKSIZE;
	strm->avail_usage = 0;
}

PiBool pi_lzf_compress_run(CompressContext * context)
{
	LZFStream *strm = (LZFStream *)context->strm;
	out_pending(context);
	in_pending(context);
	if(strm->pending == 0 && strm->avail_in == 0)
		lzf_compress_head(context);
	return TRUE;
}

compress_ret pi_lzf_compress_end(CompressContext * context)
{
	LZFStream *strm = (LZFStream *)context->strm;
	out_pending(context);
	in_pending(context);
	if(strm->pending > 0)
	{
		return OK;
	}
	if(strm->avail_usage > 0)
	{
		lzf_compress_head(context);
		return OK;
	}
	/*����������ˣ�����END*/
	pi_free(strm->buf_in);
	pi_free(strm->buf_out);
	pi_free(strm);
	return STREAM_END;
}

void *pi_lzf_uncompress_init()
{
	LZFStream *strm = pi_new0(LZFStream, 1);

	strm->avail_in = MAX_BLOCKSIZE + MAX_HDR_SIZE;
	strm->avail_usage = 0;
	strm->buf_in = pi_malloc(MAX_BLOCKSIZE + MAX_HDR_SIZE);

	strm->flushed = 0;
	strm->pending = 0;
	strm->buf_out = pi_malloc(MAX_BLOCKSIZE);
	return strm;
}

static PiBool get_compressed_data(CompressContext * context)
{
	uint cpylen;
	uint len;
	LZFStream* strm = (LZFStream*)context->strm;
	while(strm->avail_usage < 5)
	{
		if(context->avail_in == 0)
			return FALSE;
		cpylen = MIN(5 - strm->avail_usage, context->avail_in);
		pi_memcpy(&strm->buf_in[strm->avail_usage], context->next_in, cpylen);
		strm->avail_usage += cpylen;
		strm->avail_in -= cpylen;
		context->next_in = &context->next_in[cpylen];
		context->avail_in -= cpylen;
	}
	len = (strm->buf_in[3] << 8) | strm->buf_in[4];
	if(strm->buf_in[2] == 1)
		len += 2;
	len += 5;
	if(len == strm->avail_usage)
		return TRUE;
	cpylen = MIN(len - strm->avail_usage, context->avail_in);
	pi_memcpy(&strm->buf_in[strm->avail_usage], context->next_in, cpylen);
	strm->avail_usage += cpylen;
	strm->avail_in -= cpylen;
	context->next_in = &context->next_in[cpylen];
	context->avail_in -= cpylen;
	return len == strm->avail_usage ? TRUE: FALSE;
}

//����ͷ�Ľ�ѹ����
static PiBool lzf_uncompress_head(CompressContext * context)
{
	LZFStream *strm = (LZFStream *)context->strm;
	uint us;
	if(strm->buf_in[2] == 1)
	{
		us = strm->avail_usage - MAX_HDR_SIZE;
		if(us != lzf_decompress(&strm->buf_in[MAX_HDR_SIZE], strm->avail_usage - MAX_HDR_SIZE, strm->buf_out, MAX_BLOCKSIZE))
		{
			_lzf_error_set();
			return FALSE;
		}
		strm->pending = us;
	}
	else if(strm->buf_in[2] == 0)
	{
		pi_memcpy(strm->buf_out, &strm->buf_in[5], strm->avail_usage - 5);
		strm->pending = strm->avail_usage - 5;
	}
	else
	{
		pi_error_set(ERROR_TYPE_INTERNAL, 0, L"unknown blocktype", __FILE__, __LINE__);
		return FALSE;
	}
	strm->flushed = 0;
	strm->avail_in = MAX_BLOCKSIZE + MAX_HDR_SIZE;
	strm->avail_usage = 0;
	return TRUE;
}

PiBool pi_lzf_uncompress_run(CompressContext * context)
{
	LZFStream *strm = (LZFStream *)context->strm;
	out_pending(context);
	if(get_compressed_data(context))
	{
		if(strm->pending == 0)
		{
			if(!lzf_uncompress_head(context))
			{
				pi_free(strm->buf_in);
				pi_free(strm->buf_out);
				pi_free(strm);
				return FALSE;
			}
		}
	}
	return TRUE;
}

compress_ret pi_lzf_uncompress_end(CompressContext * context)
{
	LZFStream *strm = (LZFStream *)context->strm;
	out_pending(context);
	if(strm->pending > 0)
		return OK;
	if(get_compressed_data(context))
	{
		if(!lzf_uncompress_head(context))
		{
			pi_free(strm->buf_in);
			pi_free(strm->buf_out);
			pi_free(strm);
			return RUN_ERROR;
		}
		return OK;
	};
	pi_free(strm->buf_in);
	pi_free(strm->buf_out);
	pi_free(strm);
	return STREAM_END;
}