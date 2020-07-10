#include "pi_compress.h"

#include "pi_lzma.h"
#include "pi_zlib.h"
#include "pi_lzf.h"

static PiBool _arguments_check(const void * source, uint32 sourceLen, void * dest, uint32 destLen)
{
	if(NULL == source || NULL == dest)
	{
		pi_error_set(ERROR_TYPE_POINTER_NULL, 0, L"input or output buf pointer zero", __FILE__, __LINE__);
		return FALSE;
	}
	if(0 == sourceLen || 0 == destLen)
	{
		pi_error_set(ERROR_TYPE_INTERNAL, 0, L"input or output buf length zero", __FILE__, __LINE__);
		return FALSE;
	}
	return TRUE;
}

uint32 PI_API pi_compress(const void * source, uint32 sourceLen, void * dest, uint32 destLen, CompressMethod method)
{
	if(! _arguments_check(source, sourceLen, dest, destLen))
		return 0;
	switch(method)
	{
	case LZMA:
		return pi_lzma_compress(source, sourceLen, dest, destLen);
		break;
	case ZLIB:
		return pi_zlib_compress(source, sourceLen, dest, destLen);
		break;
	case LZF:
		return pi_lzf_compress(source, sourceLen, dest, destLen);
		break;
	case DEFLATE:
		return pi_deflate(source, sourceLen, dest, destLen);
		break;
	default:
		return 0;
		break;
	}
}

uint32 PI_API pi_uncompress(const void * source, uint32 sourceLen, void * dest, uint32 destLen, CompressMethod method)
{
	if(! _arguments_check(source, sourceLen, dest, destLen))
		return 0;
	switch(method)
	{
	case LZMA:
		return pi_lzma_uncompress(source, sourceLen, dest, destLen);
		break;
	case ZLIB:
		return pi_zlib_uncompress(source, sourceLen, dest, destLen);
		break;
	case LZF:
		return pi_lzf_uncompress(source, sourceLen, dest, destLen);
		break;
	case DEFLATE:
		return pi_inflate(source, sourceLen, dest, destLen);
		break;
	default:
		return 0;
		break;
	}
}

PiBool PI_API pi_compress_init(CompressContext * context)
{
	switch(context->method)
	{
	case LZMA:
		context->strm = pi_lzma_encoder_init();
		break;
	case ZLIB:
		context->strm = pi_zlib_compress_init();
		break;
	case DEFLATE:
		context->strm = pi_deflate_init();
		break;
	case LZF:
		context->strm = pi_lzf_compress_init();
		break;
	default:
		return FALSE;
		break;
	}
	if(! context->strm)
		return FALSE;
	context->avail_in = 0;
	context->avail_out = 0;
	context->next_in = 0;
	context->next_out = 0;
	context->total_in = 0;
	context->total_out = 0;
	return TRUE;
}

PiBool PI_API pi_compress_run(CompressContext * context)
{
	if((context->next_in == NULL) || (context->next_out == NULL))
	{
		pi_error_set(ERROR_TYPE_POINTER_NULL, 0, L"input or output buf pointer zero", __FILE__, __LINE__);
		return RUN_ERROR;
	}
	switch(context->method)
	{
	case LZMA:
		return pi_lzma_code_run(context);
		break;
	case ZLIB:
	case DEFLATE:
		return pi_deflate_run(context);
		break;
	case LZF:
		return pi_lzf_compress_run(context);
		break;
	default:
		return RUN_ERROR;
		break;
	}
}

compress_ret PI_API pi_compress_end(CompressContext * context)
{
	if((! context->next_in) || (! context->next_out))
	{
		pi_error_set(ERROR_TYPE_POINTER_NULL, 0, L"input or output buf pointer zero", __FILE__, __LINE__);
		return RUN_ERROR;
	}
	switch(context->method)
	{
	case LZMA:
		return pi_lzma_code_end(context);
		break;
	case ZLIB:
	case DEFLATE:
		return pi_deflate_end(context);
		break;
	case LZF:
		return pi_lzf_compress_end(context);
		break;
	default:
		return RUN_ERROR;
		break;
	}
}

PiBool PI_API pi_uncompress_init(CompressContext * context)
{
	switch(context->method)
	{
	case LZMA:
		context->strm = pi_lzma_decoder_init();
		break;
	case ZLIB:
		context->strm = pi_zlib_uncompress_init();
		break;
	case DEFLATE:
		context->strm = pi_inflate_init();
		break;
	case LZF:
		context->strm = pi_lzf_uncompress_init();
		break;
	default:
		return FALSE;
		break;
	}
	if(! context->strm)
		return FALSE;
	context->avail_in = 0;
	context->avail_out = 0;
	context->next_in = 0;
	context->next_out = 0;
	context->total_in = 0;
	context->total_out = 0;
	return TRUE;
}

PiBool PI_API pi_uncompress_run(CompressContext * context)
{
	if((! context->next_in) || (! context->next_out))
	{
		pi_error_set(ERROR_TYPE_POINTER_NULL, 0, L"input or output buf pointer zero", __FILE__, __LINE__);
		return RUN_ERROR;
	}
	switch(context->method)
	{
	case LZMA:
		return pi_lzma_code_run(context);
		break;
	case ZLIB:
	case DEFLATE:
		return pi_inflate_run(context);
		break;
	case LZF:
		return pi_lzf_uncompress_run(context);
		break;
	default:
		return FALSE;
		break;
	}
}

compress_ret PI_API pi_uncompress_end(CompressContext * context)
{
	if((!context->next_in) || (!context->next_out))
	{
		pi_error_set(ERROR_TYPE_POINTER_NULL, 0, L"input or output buf pointer zero", __FILE__, __LINE__);
		return RUN_ERROR;
	}
	switch(context->method)
	{
	case LZMA:
		return pi_lzma_code_end(context);
		break;
	case ZLIB:
	case DEFLATE:
		return pi_inflate_end(context);
		break;
	case LZF:
		return pi_lzf_uncompress_end(context);
		break;
	default:
		return RUN_ERROR;
		break;
	}
}
