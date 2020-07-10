/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 */

/**
 * ZIP压缩包解析模块，不支持ZIP64 和加密
 */

#ifndef __PI_ZIP_H__
#define __PI_ZIP_H__

#include <pi_lib.h>

typedef struct PiZip PiZip;
typedef struct PiZipDir PiZipDir;
typedef struct PiZipFile PiZipFile;

PI_BEGIN_DECLS

/**
 * 打开一个ZIP文件，并返回句柄，如果失败，则返回NULL
 * @param name ZIP文件的名称
 * @param mode 打开方式
 * @returns 返回句柄，如果失败，则返回NULL
 */
PiZip *PI_API pi_zip_open(const wchar *name, PiFileOpenMode mode);

/**
 * 关闭zip文件句柄
 * @param zip zip句柄
 * @returns 返回是否成功，失败可查错误编号
 */
PiBool PI_API pi_zip_close(PiZip *zip);

/**
 * 获取文件或目录的信息
 * @param zip zip句柄
 * @param name 文件或目录的名称
 * @param info 输出文件或目录的信息
 * @returns 返回是否成功，失败可查错误编号
 */
PiBool PI_API pi_zip_item_get_info(PiZip *zip, const wchar *name, PiFileInfo *info);

/**
 * 获取文件或目录的信息
 * @param zip zip句柄
 * @param name 文件或目录的名称
 * @returns 返回文件或目录的大小，返回-1表示失败，失败可查错误编号
 */
int64 PI_API pi_zip_item_get_size(PiZip *zip, const wchar *name);

/**
 * 判断一个目录是否为空
 * @param zip zip句柄
 * @param name 目录的名称
 * @param is_empty 返回目录是否为空
 * @returns 返回目录是否存在
 */
PiBool PI_API pi_zip_dir_is_empty(PiZip *zip, const wchar *name, PiBool *is_empty);

/**
 * 打开目录句柄
 * @param zip zip句柄
 * @param name 目录的名称
 * @returns 返回目录句柄，NULL为失败，失败可查错误编号
 */
PiZipDir *PI_API pi_zip_dir_open(PiZip *zip, const wchar *name);

/**
 * 读取目录的子项
 * @param dir 目录的句柄
 * @param index 子项索引
 * @param info 子项的信息
 * @returns 返回是否成功，失败可查错误编号
 */
PiBool PI_API pi_zip_dir_read(PiZipDir *dir, uint32 index, PiFileNameInfo *info);

/**
 * 通过文件名获取crc值
 * @param zip zip句柄
 * @param name 文件的名称
 * @param crc 输出的crc值
 * @returns 返回文件是否存在
 */
PiBool PI_API pi_zip_get_crc(PiZip *zip, const wchar *name, uint32 *crc);

/**
 * 打开文件句柄
 * @param zip zip句柄
 * @param name 文件的名称
 * @returns 文件句柄，NULL为失败，失败可查错误编号
 */
PiZipFile *PI_API pi_zip_file_open(PiZip *zip, const wchar *name);

/**
 * 获取文件大小
 * @param file 文件句柄
 * @returns 文件大小
 */
uint32 PI_API pi_zip_file_get_size(PiZipFile *file);

/**
 * 获取文件句柄获取CRC值
 * @param file 文件句柄
 * @returns crc值
 */
uint32 PI_API pi_zip_file_get_crc(PiZipFile *file);

/**
 * 读取文件数据
 * @param zip zip句柄
 * @param file 文件句柄
 * @param offset 偏移量
 * @param from_end 是否从文件尾部计算偏移
 * @param data 输出缓冲
 * @param len 输出缓冲大小
 * @returns 实际读取的数据长度，-1表示失败，失败可查错误编号
 */
sint PI_API pi_zip_file_read(PiZip *zip, const PiZipFile *file, int64 offset, PiBool from_end, char *data, uint len);

PI_END_DECLS

#endif /* __PI_ZIP_H__ */
