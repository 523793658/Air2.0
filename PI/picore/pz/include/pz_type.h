#ifndef __PZ_TYPE_H__
#define __PZ_TYPE_H__

#include <pi_lib.h>
#include <pi_compress.h>

/**
 * pz包：
 * 	    包含pzb文件块
 * pzb文件块：
 *      包含具体的文件
 *      存放文件的数据，pzb尾部是存放索引
 *      索引分为两部分：一部分索引是指向文件的索引，尾部最后26字节是文件索引的索引
 * pzb只有end_pzb才能写，其他的pzb只读
 * pzb包含的文件更新hash表，只能排序在后面的pzb中的文件去更新排序在前的pzb中的文件
 *
 * pz互斥锁：
 *		应用于合并、删除和初始化
 * pz读写锁：
 *		用于锁file_table
 * pzb互斥锁：
 *		用于锁读文件时更新缓存句柄
 *		
 * pzb块结构：
 *    ----------------------------------------------------------------------------------------------------------------------------
 *   |sizeof(int64)| sizeof(int64)| sizeof(int16)| sizeof(int8)  | sizeof(int32)| sizeof(int64)|   20字节    | 不定长| 不定长    |
 * 文|文件项目长度 | 源文件长度   | 文件名长度   | 文件压缩标志位| 文件校验码   | 文件时间信息 | 文件扩展信息| 文件名| 文件内容  |
 * 件|----------------------------------------------------------------------------------------------------------------------------
 * 项|sizeof(int64)| sizeof(int64)| sizeof(int16)| sizeof(int8)  | sizeof(int32)| sizeof(int64)|   20字节    | 不定长| 不定长    |
 * 目|文件项目长度 | 源文件长度   | 文件名长度   | 文件压缩标志位| 文件校验码   | 文件时间信息 | 文件扩展信息| 文件名| 文件内容  |
 * 区|----------------------------------------------------------------------------------------------------------------------------
 *   |                                                         .                                                                 |
 *   |                                                         .                                                                 |
 *   |                                                         .                                                                 |
 *   |----------------------------------------------------------------------------------------------------------------------------
 * 文|sizeof(int64)  | sizeof(int64)| sizeof(int64)| sizeof(int16) | sizeof(int8) | sizeof(int32)| sizeof(int64)|   20字节       |
 * 件|文件索引校验码 | 文件项目长度 | 源文件长度   | 文件名长度   | 文件压缩标志位| 文件校验码   | 文件时间信息 | 文件扩展信息   |
 * 索|----------------------------------------------------------------------------------------------------------------------------
 * 引|sizeof(int64)| 不定长 | sizeof(int64)| sizeof(int64)| sizeof(int16)| sizeof(int8)  | sizeof(int32)| sizeof(int64)| 20字节  |          
 * 区|文件偏移量   | 文件名 | 文件项目长度 | 源文件长度   | 文件名长度   | 文件压缩标志位| 文件校验码   | 文件时间信息 | 扩展信息|              
 *   |----------------------------------------------------------------------------------------------------------------------------
 *   |sizeof(int64)| 不定长 |                 ......                                                                             |
 *   |文件偏移量   | 文件名 |                 ......                                                                             |
 *   |----------------------------------------------------------------------------------------------------------------------------
 * 末|  sizeof(int8)      | sizeof(int64)| sizeof(int64)  | sizeof(int64)  | sizeof(int8) |
 * 尾|  文件索引压缩标志位| 文件索引长度 | 文件索引偏移量 | pzb拥有的文件数| end_pzb标志位|
 *    -------------------------------------------------------------------------------------
 *   末尾是指向文件索引的索引
 */
 
/* pzb末尾用于存放文件信息的索引数组的下标 */
enum {
	FILE_SIZE,           /* 文件信息+数据的字节数 */                 
	FILE_DATA_SIZE,      /* 文件数据的字节数 */
	FILE_NAME_SIZE,      /* 文件名的字节数 */
	FILE_COMPRESSION,    /* 文件是否压缩 */
	FILE_CRC,            /* 文件的CRC */
	FILE_TIME,           /* 文件创建时间 */
	FILE_EXTEND_INFO,    /* 文件扩展信息 */
	FILE_OFFSET,         /* 文件索引偏移 */
	FILE_NAME,           /* 文件名 */
	FILE_COUNT,			 /* 索引数 */
};

/* pzb索引头填充，用于判断pzb是否正常 */
#define  PZB_MASK        -1             /* 0xFFFFFFFF */
#define  PZB_MASK_SIZE   8              /* 0xFFFFFFFF */

#define  FLOAT_ZERO      0.00001        /* 用于判断FLOAT是否为0 */

/* 文件索引各部分所需要的字节数 */
#define FILE_SIZE_INDEX_LEN             sizeof(int64)                  /* 文件索引之文件长度字节 */
#define FILE_DATA_SIZE_INDEX_LEN        sizeof(int64)                  /* 文件索引之文件数据字节 */
#define FILE_NAME_SIZE_INDEX_LEN        sizeof(int16)                  /* 文件索引之文件名字节 */
#define FILE_COMPRESSION_INDEX_LEN      sizeof(int8)                   /* 文件索引之压缩标志位字节 */
#define FILE_CRC_INDEX_LEN              sizeof(int32)                  /* 文件索引之CRC字节 */
#define FILE_TIME_INDEX_LEN             sizeof(int64)                  /* 文件索引之存储时间字节 */
#define FILE_OFFSET_INDEX_LEN           sizeof(int64)                  /* 文件索引之偏移量存储字节 */

#define INDEX_NONAME			(FILE_SIZE_INDEX_LEN\
								+ FILE_DATA_SIZE_INDEX_LEN\
								+ FILE_NAME_SIZE_INDEX_LEN\
								+ FILE_COMPRESSION_INDEX_LEN\
								+ FILE_CRC_INDEX_LEN\
								+ FILE_TIME_INDEX_LEN\
								+ FILE_OFFSET_INDEX_LEN\
								+ EXTEND_INFO_NUM)                                 /* 文件的索引长度，不含文件名 */

#define INDEX_NONAME_NOOFFSET	(INDEX_NONAME - FILE_OFFSET_INDEX_LEN)             /* 文件的索引长度，不含文件名和偏移量 */
#define PZB_SIZE				(40 * 1024 * 1024)                                 /* pzb数据的大小上限 */
#define PZB_FILE_COUNT			100                                                /* pzb包含的文件数量上限 */
#define PZB_MERGE_SCALE			0.5                                                /* pzb有效数据低于该阀值就合并 */
#define COMPRESS_RATIO			1                                                  /* 期望压缩长度比例 */
#define END_SIZE				26                                                 /* pzb尾部文件索引的索引的大小 */
#define CACHE_PZB_NUM           1                                                  /* PZB缓冲个数 */

typedef struct 
{
	PiBool is_compress;      /* 索引是否被压缩 */
	int8 is_end;             /* 是否是end_pzb */
	int64 size;              /* 索引的大小 */
	int64 offset;            /* 索引的偏移 */
	int64 count;             /* pzb拥有文件个数 */
}IndexInfo;

typedef struct
{
	int64 is_merge;                 /* 判断该pzb是否合并 */
	int64 offset;				    /* 块中文件的索引起始位置（以FFFFFFFFFFFFFFFF为标识）*/	
	int64 total_size;				/* 块的总大小 */
	int64 usefull_size;				/* 块的有用部分的大小 */
	int64 index;              		/* 块的编号 */
	int32 total_file_count;			/* 块中文件的总个数 */
	float proportion;				/* 有用的大小占总大小的比例 */
	int32 usefull_file_count;		/* 块中有用文件的个数 */
	int64 major_version;            /* 根据名字而来的主版本号，用于保证pzb的顺序 */
	int64 minor_version;            /* 根据名字而来的次版本号，用于保证pzb的顺序 */
	wchar pzb_path[NAME_LENGTH];    /* 块的名字，组成格式：主版本号.次版本号 */
	void *file_handle;				/* 块的写文件指针，一般为空，只有最后一块保留 */
	sint spin_lock;                 /* 自旋锁，用于读文件更新缓存时 */
	sint read_count;                /* 该pzb被读取的次数 */
}PZB;

typedef struct
{
	char file_name[NAME_LENGTH];	/* 文件名 */
	PZB *pzb;						/* 文件所对应的pzb块的指针 */
	float index;					/* 文件所在的块的编号 */
	int64 offset;					/* 文件在块中的偏移位置 */
	int64 file_size;				/* 源文件的长度 */
	int64 length;					/* pzb中每个文件项目的长度 */	
	int64 index_offset;				/* 文件的索引所在的索引偏移 */
	int name_length;				/* 文件名长度 */
	int crc;						/* 文件的校验码 */
	int64 time;						/* 文件的时间信息 */	
	int8 compression;				/* 文件的压缩信息 */
	int8 encryption;				/* 文件加密信息 */
	PiExtendInfo extend_info;		/* 扩展信息字段 */
}FileInfo;

typedef struct 
{
	int pzb_count;   				/* pz中现有pzb块的个数 */
	PiBool read_only;               /* 是否为只读目录 */
	wchar path[NAME_LENGTH];     	/* pz目录，最后保留的是可读写的目录 */
	PiVector pzb_vector;			/* 块列表，当一个pzb不再作为end_pzb时，就推入pzb_vector中 */ 
	PiDhash file_table;				/* 文件列表 */
	PZB *end_pzb;					/* 最后一块，唯一有写操作的pzb */
	PiMutex* lock;					/* 合并、删除和PZ初始化加锁 */
	sint rlock;						/* 读锁和下面的wlock构成读写锁，重点锁file_table */
	sint wlock;						/* 写锁 */
	sint synchronized;              /* 同步锁，由于多线程读取pz文件，会导致文件读取失败*/
	PZB *cache_pzb[CACHE_PZB_NUM];  /* pzb缓冲数组 */
	PiBytes bytes[FILE_COUNT];	    /* 写文件时候的缓冲字节数组，每个缓冲对应文件的一类信息 */
}PZ;

typedef struct  
{
	PiVector pzblist;			    /* 待合并的pzb */
	PiVector filelist;              /* 待合并的文件 */
}MergeData;

#endif /* __PZ_TYPE_H__ */
