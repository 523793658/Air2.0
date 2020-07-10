
#include "lzma.h"
#include "compress.h"

//lzma库的错误码处理
static void _lzme_set_error(lzma_ret ret)
{
	const wchar *msg;
	switch (ret)
	{
	case LZMA_UNSUPPORTED_CHECK:
		msg = L"Specified integrity check is not supported";
		break;
	case LZMA_MEM_ERROR:
		msg = L"Memory allocation failed";
		break;
	case LZMA_FORMAT_ERROR:
		msg = L"The input is not in the .xz format";
		break;
	case LZMA_OPTIONS_ERROR:
		msg = L"Unsupported compression options";
		break;
	case LZMA_DATA_ERROR:
		msg = L"Compressed file is corrupt";
		break;
	case LZMA_BUF_ERROR:
		msg = L"Compressed file is truncated or otherwise corrupt";
		break;
	default:
		msg = L"Unknown error, possibly a bug";
		break;
	}
	pi_error_set(ERROR_TYPE_INTERNAL, ret, msg, __FILE__, __LINE__);
}

//一次调用lzma压缩接口，返回压缩后的长度，返回0表示出错，出错可查错误码
uint32 pi_lzma_compress(const void * source, uint32 sourceLen, void * dest, uint32 destLen)
{
	lzma_ret ret;
	lzma_stream strm = LZMA_STREAM_INIT;
	lzma_filter filters[3];
	lzma_options_lzma opt_lzma2;
	lzma_lzma_preset(&opt_lzma2, LZMA_PRESET_DEFAULT);
	filters[0].id = LZMA_FILTER_X86;
	filters[0].options = NULL;
	filters[1].id = LZMA_FILTER_LZMA2;
	filters[1].options = &opt_lzma2;
	filters[2].id = LZMA_VLI_UNKNOWN;
	filters[2].options = NULL;
	ret = lzma_stream_encoder(&strm, filters, LZMA_CHECK_CRC64);
	if(LZMA_OK != ret)
	{
		lzma_end(&strm);
		_lzme_set_error(ret);
		return 0;
	}
	strm.next_out = dest;
	strm.avail_out = destLen;
	strm.next_in = source;
	strm.avail_in = sourceLen;
	ret = lzma_code(&strm, LZMA_FINISH);
	lzma_end(&strm);
	if(LZMA_STREAM_END != ret)
	{
		if(LZMA_OK == ret)
			ret = LZMA_BUF_ERROR;
		_lzme_set_error(ret);
		return 0;
	}
	return (uint32)strm.total_out;
}

//一次调用lzma解压接口，返回解压后的长度，返回0表示出错，出错可查错误码
uint32 pi_lzma_uncompress(const void * source, uint32 sourceLen, void * dest, uint32 destLen)
{
	lzma_stream strm = LZMA_STREAM_INIT;
	lzma_ret ret = lzma_stream_decoder(&strm, UINT64_MAX, LZMA_CONCATENATED);
	if(LZMA_OK != ret)
	{
		lzma_end(&strm);
		_lzme_set_error(ret);
		return 0;
	}
	strm.next_out = dest;
	strm.avail_out = destLen;
	strm.next_in = source;
	strm.avail_in = sourceLen;
	ret = lzma_code(&strm, LZMA_FINISH);
	lzma_end(&strm);
	if(LZMA_STREAM_END != ret)
	{
		if(LZMA_OK == ret)
			ret = LZMA_BUF_ERROR;
		_lzme_set_error(ret);
		return 0;
	}
	return (uint32)strm.total_out;
}

//解压初始化，返回lzmaRUN需要的数据结构的指针，返回NULL表示出错，出错可查错误码
void * pi_lzma_decoder_init()
{
	lzma_stream * strm = pi_new0(lzma_stream, 1);
	lzma_ret ret = lzma_stream_decoder(strm, UINT64_MAX, LZMA_CONCATENATED);
	if(LZMA_OK != ret)
	{
		pi_free(strm);
		_lzme_set_error(ret);
		return NULL;
	}
	return strm;
}

//压缩初始化，返回lzmaRUN需要的数据结构的指针，返回NULL表示出错，出错可查错误码
void * pi_lzma_encoder_init()
{
	lzma_ret ret;
	lzma_filter filters[3];
	lzma_options_lzma opt_lzma2;
	lzma_stream *strm = pi_new0(lzma_stream, 1);
	lzma_lzma_preset(&opt_lzma2, LZMA_PRESET_DEFAULT);
	filters[0].id = LZMA_FILTER_X86;
	filters[0].options = NULL;
	filters[1].id = LZMA_FILTER_LZMA2;
	filters[1].options = &opt_lzma2;
	filters[2].id = LZMA_VLI_UNKNOWN;
	filters[2].options = NULL;
	ret = lzma_stream_encoder(strm, filters, LZMA_CHECK_CRC64);
	if (LZMA_OK != ret)
	{
		pi_free(strm);
		_lzme_set_error(ret);
		return NULL;
	}
	return strm;
}

static void * _get_stream(CompressContext *context)
{
	lzma_stream *strm = (lzma_stream *)context->strm;
	strm->next_in = (const uint8_t *)context->next_in;
	strm->avail_in = context->avail_in;
	strm->next_out = (uint8_t *)context->next_out;
	strm->avail_out = context->avail_out;
	return strm;
}

static void _set_stream(CompressContext *context)
{
	lzma_stream * strm = (lzma_stream *)context->strm;
	context->total_in = (uint)strm->total_in;
	context->total_out = (uint)strm->total_out;
	context->next_in = (char *)strm->next_in;
	context->avail_in = strm->avail_in;
	context->next_out = (char *)strm->next_out;
	context->avail_out = strm->avail_out;
}

PiBool pi_lzma_code_run(CompressContext *context)
{
	lzma_ret ret;
	lzma_stream *strm = _get_stream(context);
	ret = lzma_code(strm, LZMA_RUN);
	if (LZMA_OK != ret && LZMA_STREAM_END != ret)
	{
		lzma_end(strm);
		pi_free(strm);
		_lzme_set_error(ret);
		return FALSE;
	}
	_set_stream(context);
	return TRUE;
}

compress_ret pi_lzma_code_end(CompressContext *context)
{
	lzma_ret ret;
	lzma_stream * strm = _get_stream(context);
	ret = lzma_code(strm, LZMA_FINISH);
	if (LZMA_STREAM_END == ret)
	{
		_set_stream(context);
		lzma_end(strm);
		pi_free(strm);
		return STREAM_END;
	}
	if(LZMA_OK == ret)
	{
		_set_stream(context);
		return OK;
	}
	lzma_end(strm);
	pi_free(strm);
	return RUN_ERROR;
}
