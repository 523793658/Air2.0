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
 * ZIP文件格式的一些数据定义，参考 https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT
 */

#ifndef __ZIP_FORMAT_H__
#define __ZIP_FORMAT_H__

#include <pi_lib.h>

#define LOCAL_FILE_HEADER_SIGNATURE (0x04034b50)						/* 分文件头信息标志 */
#define DATA_DESCRIPTOR_SIGNATURE (0x08074b50)							/* 分文件头信息标志 */
#define FILE_HEADER_SIGNATURE (0x02014b50)								/* 中心目录结构文件头信息标志 */
#define END_OF_CENTRAL_DIRECTORY_RECORD_SIGNATURE (0x06054b50)			/* 中心目录记录结束符的信息标志 */

#define CENTRAL_DIRECTORY_HEADER_WITH_SIGNATURE_SIZE (46)				/* central directory file header 包含signature的基本长度 */
#define CENTRAL_DIRECTORY_HEADER_SIZE (42)								/* central directory file header 不包含signature的基本长度 */
#define LOCAL_FILE_HEADER_WITH_SIGNATURE_SIZE (30)						/* local file header 包含signature的基本长度 */
#define LOCAL_FILE_HEADER_SIZE (26)										/* local file header 不包含signature的基本长度 */
#define EOCD_RECORD_SIZE (22)											/* end of central directory record 不包含signature的基本长度 */

typedef enum
{
	ZIP_SYSTEM_MSDOS_OS2_FAT,											/* MS-DOS and OS/2 (FAT/VFAT/FAT32 file systems) */
	ZIP_SYSTEM_AMIGA,													/* Amiga */
	ZIP_SYSTEM_OPENVMS,													/* OpenVMS */
	ZIP_SYSTEM_UNIX,													/* UNIX */
	ZIP_SYSTEM_VM_CMS,													/* VM/CMS */
	ZIP_SYSTEM_ATARI_ST,												/* Atari ST */
	ZIP_SYSTEM_OS2_HPFS,												/* OS/2 H.P.F.S. */
	ZIP_SYSTEM_MACINTOSH,												/* Macintosh */
	ZIP_SYSTEM_Z_SYSTEM,												/* Z-System */
	ZIP_SYSTEM_CPM,														/* CP/M */
	ZIP_SYSTEM_WINDOWS_NTFS,											/* Windows NTFS */
	ZIP_SYSTEM_MVS,														/* MVS(OS/390 - Z/OS) */
	ZIP_SYSTEM_VSE,														/* VSE */
	ZIP_SYSTEM_ACORN_RISC,												/* Acorn Risc */
	ZIP_SYSTEM_VFAT,													/* VFAT */
	ZIP_SYSTEM_ALTERNATE_MVS,											/* alternate MVS */
	ZIP_SYSTEM_BEOS,													/* BeOS */
	ZIP_SYSTEM_TANDEM,													/* Tandem */
	ZIP_SYSTEM_OS_400,													/* OS/400 */
	ZIP_SYSTEM_OS_X														/* OS X(Darwin) */
} ZipSystem;

/* Compression Method, only 0(store) and 8(deflate) are supported here */
typedef enum
{
	ZIP_METHOD_STORE,													/* The file is stored (no compression) */
	ZIP_METHOD_SHRINK,													/* The file is Shrunk */
	ZIP_METHOD_REDUCE1,													/* The file is Reduced with compression factor 1 */
	ZIP_METHOD_REDUCE2,													/* The file is Reduced with compression factor 2 */
	ZIP_METHOD_REDUCE3,													/* The file is Reduced with compression factor 3 */
	ZIP_METHOD_REDUCE4,													/* The file is Reduced with compression factor 4 */
	ZIP_METHOD_IMPLODE,													/* The file is Imploded */
	ZIP_METHOD_DEFLATE,													/* The file is Deflated */
	ZIP_METHOD_DEFLATE64,												/* Enhanced Deflating using Deflate64(tm) */
	ZIP_METHOD_PKWARE_IMPLODE = 10,										/* PKWARE Data Compression Library Imploding(old IBM TERSE) */
	ZIP_METHOD_BZIP2 = 12,												/* File is compressed using BZIP2 algorithm */
	ZIP_METHOD_LZMA = 14,												/* LZMA(EFS) */
	ZIP_METHOD_IBM_TERSE = 18,											/* File is compressed using IBM TERSE(new) */
	ZIP_METHOD_IBM_LZ77 = 19,											/* IBM LZ77 z Architecture(PFS) */
	ZIP_METHOD_WAVPACK = 97,											/* WavPack compressed data */
	ZIP_METHOD_PPMD = 98,												/* PPMd version I, Rev 1 */
	ZIP_METHOD_DEFAULT = 0xffff											/* The file is stored (no compression) */
} ZipCompressionMethod;

typedef enum
{
	ZIP_ENCRYPTED = 0x0001,												/* indicates that the file is encrypted. */
	ZIP_IMPLODE_SLIDING = 0x0002,										/* if set, indicates an 8K sliding dictionary was used.  If clear, then a 4K sliding dictionary was used */
	ZIP_IMPLODE_SHANNON_FANO_TREES = 0x0004,							/* if set, indicates 3 Shannon-Fano trees were used to encode the sliding dictionary output.  If clear, then 2 Shannon-Fano trees were used. */
	ZIP_DEFLATE_NORMAL = 0x0000,										/* Normal (-en) compression option was used. */
	ZIP_DEFLATE_EXTRA = 0x0002,											/* Maximum (-exx/-ex) compression option was used */
	ZIP_DEFLATE_FAST = 0x0004,											/* Fast (-ef) compression option was used */
	ZIP_DEFLATE_SUPERFAST = 0x0006,										/* Super Fast (-es) compression option was used */
	ZIP_DEFLATE_MASK = 0x0006,											/* For Deflating, Bit 2 and Bit 1 indicates compression option */
	ZIP_LZMA_MASK = 0x0002,												/* For LZMA, Bit 1 indicates an end-of-stream (EOS) marker is used to mark the end of the compressed data stream.  */
	ZIP_SUMS_FOLLOW = 0x0008,											/* crc and sizes come after the data */
	ZIP_ENHANCED_DEFLATIONG_RESERVED = 0x0010,							/* Reserved for use with method 8, for enhanced deflating. */
	ZIP_PATCH = 0x0020,													/* this indicates that the file is compressed patched data */
	ZIP_STRONG_ENCRYPTION = 0x0040,
	ZIP_UNUSED = 0x0780,												/* Bit 7 8 9 10 Currently unused */
	ZIP_LANGUAGE_ENCODEING_FLAG = 0x0800,								/* If this bit is set, the filename and comment fields for this file MUST be encoded using UTF-8. */
	ZIP_ENHANCED_COMPRESSION_RESERVED = 0x1000,							/* Reserved by PKWARE for enhanced compression */
	ZIP_LOCAL_HEADER_HIDE = 0x2000,										/* Set when encrypting the Central Directory to indicate selected data values in the Local Header are masked to hide their actual values */
	ZIP_RESERVED = 0xC000												/* Bit 14 15 Reserved by PKWARE */
} ZipFlags;

/* Dos/Win file attributes */
typedef enum
{
	ZIP_A_RDONLY = 0x01,
	ZIP_A_HIDDEN = 0x02,
	ZIP_A_SYSTEM = 0x04,
	ZIP_A_SUBDIR = 0x10,
	ZIP_A_ARCH = 0x20,
	ZIP_A_MASK = 0x37
} ZipAttributes;

typedef struct
{
	/* uint32 signature; */												/* local file header signature(0x04034b50) */
	uint16 version_needed;												/* version needed to extract */
	uint16 flag;														/* general purpose bit flag */
	uint16 compression_method;											/* compression method */
	uint16 time;														/* File last modification time */
	uint16 date;														/* File last modification date */
	uint32 crc32;														/* crc-32 */
	uint32 compressed_size;												/* compressed size */
	uint32 uncompressed_size;											/* uncompressed size */
	uint16 file_name_length;											/* filename length(null if stdin) */
	uint16 extra_field_length;											/* extra field length */
	/* followed by filename(of variable size) */
	/* followed by extra field(of variable size) */
	/* followed by compressed_data(of variable size) */
} LocalFileHeader;

/* 如果LocalFileHeader的flag的第三位被设置了，则LocalFileHeader在压缩数据之后会有一个DataDescriptor，LocalFileHeader的crc32、compressed_size、uncompressed_size等值为0 */
typedef struct
{
	/* uint32 signature; */												/* Optional data descriptor signature = 0x08074b50 */
	uint32 crc32;														/* crc-32 */
	uint32 compressed_size;												/* compressed size */
	uint32 uncompressed_size;											/* uncompressed size */
} DataDescriptor;

typedef struct
{
	/* uint32 signature; */												/* central file header signature(0x02014b50) */
	uint16 version;														/* version made by */
	uint16 version_needed;												/* version need to extract */
	uint16 flag;														/* general purpose bit flag */
	uint16 compression_method;											/* compression method */
	uint16 time;														/* File last modification time */
	uint16 date;														/* File last modification date */
	uint32 crc32;														/* crc-32 */
	uint32 compressed_size;												/* compressed size */
	uint32 uncompressed_size;											/* uncompressed size */
	uint16 file_name_length;											/* filename length(null if stdin) */
	uint16 extra_field_length;											/* extra field length */
	uint16 file_comment_length;											/* file comment length */
	uint16 disk_number_start;											/* disk number of start(if spanning zip over multiple disks) */
	uint16 internal_attributes;											/* internal file attributes, bit0 = ascii */
	uint32 external_attributes;											/* external file attributes, eg. msdos attrib byte */
	uint32 header_offset;												/* relative offset of local file header, seekval if singledisk */
	/* followed by filename(of variable size) */
	/* followed by extra field(of variable size) */
	/* followed by file comment(of variable size) */
} CentralDirectoryHeader;

/* end of central dir record 中心目录结束标记，注释掉的字段为暂时不需要的字段 */
typedef struct
{
	/* uint32 signature; */												/* end of central directory signature = 0x06054b50 */
	uint16 disk_number;													/* number of this disk */
	uint16 start_disk_number;											/* number of the disk with the start of the central dir */
	uint16 entries_on_disk;												/* total number of entries in the central dir on this disk */
	uint16 entries_in_directory;										/* total number of entries in the central dir */
	uint32 directory_size;												/* size of the central directory */
	uint32 directory_offset;											/* offset of start of central directory with respect to the starting disk number */
	uint16 comment_length;												/* zip file comment length */
	/* followed by comment(of variable size) */
} EndOfCentralDirectoryRecord;

#endif /* __ZIP_FORMAT_H__ */
