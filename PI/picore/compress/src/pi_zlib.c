
#include "zlib.h"
#include "compress.h"

//zlib库的错误码处理
static void _zlib_error_set(sint err)
{
	wchar *msg = pi_str_to_wstr(zError(err), PI_CP_GBK);
	pi_error_set(ERROR_TYPE_INTERNAL, err, msg, __FILE__, __LINE__);
	pi_free(msg);
}

//带zlib头的一次压缩接口，返回压缩后的长度，0表示出错，出错可查错误码
uint32 pi_zlib_compress(const void *source, uint32 sourceLen, void *dest, uint32 destLen)
{
	sint err = compress(dest, (uLongf *)&destLen, source, sourceLen);
	if(err != Z_OK)
	{
		_zlib_error_set(err);
		return 0;
	}
	return destLen;
}

//带zlib头的一次解压接口，返回解压后的长度，0表示出错，出错可查错误码
uint32 pi_zlib_uncompress(const void * source, uint32 sourceLen, void *dest, uint32 destLen)
{
	sint err = uncompress(dest, (uLongf *)&destLen, source, sourceLen);
	if(err != Z_OK)
	{
		_zlib_error_set(err);
		return 0;
	}
	return destLen;
}

//不带zlib头的纯deflate算法一次压缩接口，返回错误码，内部接口
static sint _deflate(char *dest, uint *destLen, const char *source, uint sourceLen)
{
	z_stream stream;
	sint err;
	stream.next_in = (Bytef *)source;
	stream.avail_in = sourceLen;
	stream.next_out = (Bytef *)dest;
	stream.avail_out = *destLen;
	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;
	stream.opaque = (voidpf)0;
	err = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
	if (err != Z_OK)
		return err;
	err = deflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END)
	{
		deflateEnd(&stream);
		return err == Z_OK ? Z_BUF_ERROR : err;
	}
	*destLen = stream.total_out;
	err = deflateEnd(&stream);
	return err;
}

//不带zlib头的纯deflate算法一次压缩接口，返回压缩后的长度，0表示出错，出错可查错误码
uint32 pi_deflate(const void *source, uint32 sourceLen, void *dest, uint32 destLen)
{
	sint err = _deflate(dest, &destLen, source, sourceLen);
	if(err != Z_OK)
	{
		_zlib_error_set (err);
		return 0;
	}
	return destLen;
}

//不带zlib头的纯inflate算法一次调用解压接口，返回错误码，内部接口
static sint _inflate(char *dest, uint *destLen, const char *source, uint sourceLen)
{
	z_stream stream;
	sint err;
	stream.next_in = (Bytef *)source;
	stream.avail_in = sourceLen;
	stream.next_out = (Bytef *)dest;
	stream.avail_out = *destLen;
	stream.zalloc = (alloc_func)0;
	stream.zfree = (free_func)0;
	err = inflateInit2(&stream, -15);
	if (err != Z_OK)
		return err;
	err = inflate(&stream, Z_FINISH);
	if (err != Z_STREAM_END)
	{
		inflateEnd(&stream);
		if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0))
			return Z_DATA_ERROR;
		return err;
	}
	*destLen = stream.total_out;
	err = inflateEnd(&stream);
	return err;
}

//不带zlib头的纯inflate算法一次调用解压接口，返回解压后的长度，0表示出错，出错可查错误码
uint32 pi_inflate(const void * source, uint32 sourceLen, void * dest, uint32 destLen)
{
	sint err = _inflate(dest, &destLen, source, sourceLen);
	if(err != Z_OK)
	{
		_zlib_error_set(err);
		return 0;
	}
	return destLen;
}

void * pi_zlib_compress_init()
{
	sint err;
	z_stream * stream = pi_new(z_stream, 1);
	stream->zalloc = (alloc_func)0;
	stream->zfree = (free_func)0;
	stream->opaque = (voidpf)0;
	err = deflateInit(stream, Z_DEFAULT_COMPRESSION);
	if (err != Z_OK)
	{
		_zlib_error_set(err);
		pi_free(stream);
		return NULL;
	}
	return stream;
}

void * pi_deflate_init()
{
	sint err;
	z_stream * stream = pi_new(z_stream, 1);
	stream->zalloc = (alloc_func)0;
	stream->zfree = (free_func)0;
	stream->opaque = (voidpf)0;
	err = deflateInit2(stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
	if (err != Z_OK)
	{
		_zlib_error_set(err);
		pi_free(stream);
		return NULL;
	}
	return stream;
}

void * pi_zlib_uncompress_init()
{
	sint err;
	z_stream * stream = pi_new(z_stream, 1);
	stream->zalloc = (alloc_func)0;
	stream->zfree = (free_func)0;
	err = inflateInit(stream);
	if (err != Z_OK)
	{
		_zlib_error_set(err);
		pi_free(stream);
		return NULL;
	}
	return stream;
}

void * pi_inflate_init()
{
	sint err;
	z_stream * stream = pi_new(z_stream, 1);
	stream->zalloc = (alloc_func)0;
	stream->zfree = (free_func)0;
	err = inflateInit2(stream, -15);
	if (err != Z_OK)
	{
		_zlib_error_set(err);
		pi_free(stream);
		return NULL;
	}
	return stream;
}

static z_stream * _get_stream(CompressContext * context)
{
	z_stream * stream = (z_stream *)context->strm;
	stream->next_in = (Bytef *)context->next_in;
	stream->avail_in = context->avail_in;
	stream->next_out = (Bytef *)context->next_out;
	stream->avail_out = context->avail_out;
	return stream;
}

static void _set_stream(CompressContext * context)
{
	z_stream * stream = (z_stream *)context->strm;
	context->total_in = stream->total_in;
	context->total_out = stream->total_out;
	context->next_in = (char *)stream->next_in;
	context->avail_in = stream->avail_in;
	context->next_out = (char *)stream->next_out;
	context->avail_out = stream->avail_out;
}

PiBool pi_deflate_run(CompressContext * context)
{
	sint err;
	z_stream * stream = _get_stream(context);
	err = deflate(stream, Z_NO_FLUSH);
	if(err != Z_OK && err != Z_STREAM_END)
	{
		_zlib_error_set(err);
		inflateEnd(stream);
		pi_free(stream);
		return FALSE;
	}
	_set_stream(context);
	return TRUE;
}

compress_ret pi_deflate_end(CompressContext * context)
{
	sint err;
	z_stream * stream = _get_stream(context);
	err = deflate(stream, Z_FINISH);
	if(err == Z_STREAM_END)
	{
		err = deflateEnd(stream);
		if(Z_OK != err)
		{
			_zlib_error_set(err);
			return RUN_ERROR;
		}
		_set_stream(context);
		pi_free(stream);
		return STREAM_END;
	}
	if(err == Z_OK)
	{
		_set_stream(context);
		return OK;
	}
	_zlib_error_set(err);
	deflateEnd(stream);
	pi_free(stream);
	return RUN_ERROR;
}

PiBool pi_inflate_run(CompressContext *context)
{
	sint err;
	z_stream * stream = _get_stream(context);
	err = inflate(stream, Z_NO_FLUSH);
	if(err != Z_OK && err != Z_STREAM_END)
	{
		_zlib_error_set(err);
		inflateEnd(stream);
		pi_free(stream);
		return FALSE;
	}
	_set_stream(context);
	return TRUE;
}

compress_ret pi_inflate_end(CompressContext * context)
{
	sint err;
	z_stream * stream = _get_stream(context);
	err = inflate(stream, Z_FINISH);
	if(err == Z_STREAM_END)
	{
		err = inflateEnd(stream);
		if(Z_OK != err)
		{
			_zlib_error_set(err);
			return RUN_ERROR;
		}
		_set_stream(context);
		pi_free(stream);
		return STREAM_END;
	}
	if(err == Z_OK)
	{
		_set_stream(context);
		return OK;
	}
	_zlib_error_set(err);
	inflateEnd(stream);
	pi_free(stream);
	return RUN_ERROR;
}