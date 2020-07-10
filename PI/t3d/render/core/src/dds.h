#ifndef INCLUDE_IMAGE_DDS_H
#define INCLUDE_IMAGE_DDS_H

#include <pi_lib.h>
#include <image.h>

PI_BEGIN_DECLS 

/**
 * 判断文件格式是否DDS
 */
PiBool PI_API dds_is_valid(byte *data, uint size);

/**
 * 加载DDS
 */
PiBool PI_API dds_load(PiImage *image, byte *data, uint size);

PI_END_DECLS 

#endif /* INCLUDE_IMAGE_DDS_H */