#ifndef __PI_COMPRESS_H__
#define __PI_COMPRESS_H__

#include "compress.h"

PI_BEGIN_DECLS

uint32 PI_API pi_compress(const void * source, uint32 sourceLen, void * dest, uint32 destLen, CompressMethod method);

uint32 PI_API pi_uncompress(const void * source, uint32 sourceLen, void * dest, uint32 destLen, CompressMethod method);

PiBool PI_API pi_compress_init(CompressContext * context);

PiBool PI_API pi_compress_run(CompressContext * context);

compress_ret PI_API pi_compress_end(CompressContext * context);

PiBool PI_API pi_uncompress_init(CompressContext * context);

PiBool PI_API pi_uncompress_run(CompressContext * context);

compress_ret PI_API pi_uncompress_end(CompressContext * context);

PI_END_DECLS

#endif /* __PI_COMPRESS_H__ */