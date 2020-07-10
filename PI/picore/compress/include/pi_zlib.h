#ifndef __PI_ZLIB_H__
#define __PI_ZLIB_H__

#include "compress.h"

PI_BEGIN_DECLS

uint32 pi_zlib_compress(const void * source, uint32 sourceLen, void * dest, uint32 destLen);

uint32 pi_zlib_uncompress(const void * source, uint32 sourceLen, void * dest, uint32 destLen);

uint32 pi_deflate(const void * source, uint32 sourceLen, void * dest, uint32 destLen);

uint32 pi_inflate(const void * source, uint32 sourceLen, void * dest, uint32 destLen);

void * pi_zlib_compress_init();

void * pi_deflate_init();

PiBool pi_deflate_run(CompressContext * context);

compress_ret pi_deflate_end(CompressContext * context);

void * pi_zlib_uncompress_init();

void * pi_inflate_init();

PiBool pi_inflate_run(CompressContext * context);

compress_ret pi_inflate_end(CompressContext * context);

PI_END_DECLS

#endif /* __PI_ZLIB_H__ */