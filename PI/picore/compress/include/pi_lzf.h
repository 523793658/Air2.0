#ifndef __PI_LZF_H__
#define __PI_LZF_H__

#include "pi_lib.h"
#include "compress.h"

PI_BEGIN_DECLS

uint32 pi_lzf_uncompress(const void * source, uint32 sourceLen, void * dest, uint32 destLen);

uint32 pi_lzf_compress(const void * source, uint32 sourceLen, void * dest, uint32 destLen);

void * pi_lzf_compress_init();

PiBool pi_lzf_compress_run(CompressContext * context);

compress_ret pi_lzf_compress_end(CompressContext * context);

void * pi_lzf_uncompress_init();

PiBool pi_lzf_uncompress_run(CompressContext * context);

compress_ret pi_lzf_uncompress_end(CompressContext * context);

PI_END_DECLS

#endif /* __PI_LZF_H__ */