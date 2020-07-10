#ifndef __PI_LZMA_H__
#define __PI_LZMA_H__

#include "compress.h"

PI_BEGIN_DECLS

uint32 pi_lzma_compress(const void * source, uint32 sourceLen, void * dest, uint32 destLen);

uint32 pi_lzma_uncompress(const void * source, uint32 sourceLen, void * dest, uint32 destLen);

void * pi_lzma_decoder_init();

void * pi_lzma_encoder_init();

PiBool pi_lzma_code_run(CompressContext * context);

compress_ret pi_lzma_code_end(CompressContext * context);

PI_END_DECLS

#endif /* __PI_LZMA_H__ */