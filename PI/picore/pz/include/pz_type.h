#ifndef __PZ_TYPE_H__
#define __PZ_TYPE_H__

#include <pi_lib.h>
#include <pi_compress.h>

/**
 * pz����
 * 	    ����pzb�ļ���
 * pzb�ļ��飺
 *      ����������ļ�
 *      ����ļ������ݣ�pzbβ���Ǵ������
 *      ������Ϊ�����֣�һ����������ָ���ļ���������β�����26�ֽ����ļ�����������
 * pzbֻ��end_pzb����д��������pzbֻ��
 * pzb�������ļ�����hash��ֻ�������ں����pzb�е��ļ�ȥ����������ǰ��pzb�е��ļ�
 *
 * pz��������
 *		Ӧ���ںϲ���ɾ���ͳ�ʼ��
 * pz��д����
 *		������file_table
 * pzb��������
 *		���������ļ�ʱ���»�����
 *		
 * pzb��ṹ��
 *    ----------------------------------------------------------------------------------------------------------------------------
 *   |sizeof(int64)| sizeof(int64)| sizeof(int16)| sizeof(int8)  | sizeof(int32)| sizeof(int64)|   20�ֽ�    | ������| ������    |
 * ��|�ļ���Ŀ���� | Դ�ļ�����   | �ļ�������   | �ļ�ѹ����־λ| �ļ�У����   | �ļ�ʱ����Ϣ | �ļ���չ��Ϣ| �ļ���| �ļ�����  |
 * ��|----------------------------------------------------------------------------------------------------------------------------
 * ��|sizeof(int64)| sizeof(int64)| sizeof(int16)| sizeof(int8)  | sizeof(int32)| sizeof(int64)|   20�ֽ�    | ������| ������    |
 * Ŀ|�ļ���Ŀ���� | Դ�ļ�����   | �ļ�������   | �ļ�ѹ����־λ| �ļ�У����   | �ļ�ʱ����Ϣ | �ļ���չ��Ϣ| �ļ���| �ļ�����  |
 * ��|----------------------------------------------------------------------------------------------------------------------------
 *   |                                                         .                                                                 |
 *   |                                                         .                                                                 |
 *   |                                                         .                                                                 |
 *   |----------------------------------------------------------------------------------------------------------------------------
 * ��|sizeof(int64)  | sizeof(int64)| sizeof(int64)| sizeof(int16) | sizeof(int8) | sizeof(int32)| sizeof(int64)|   20�ֽ�       |
 * ��|�ļ�����У���� | �ļ���Ŀ���� | Դ�ļ�����   | �ļ�������   | �ļ�ѹ����־λ| �ļ�У����   | �ļ�ʱ����Ϣ | �ļ���չ��Ϣ   |
 * ��|----------------------------------------------------------------------------------------------------------------------------
 * ��|sizeof(int64)| ������ | sizeof(int64)| sizeof(int64)| sizeof(int16)| sizeof(int8)  | sizeof(int32)| sizeof(int64)| 20�ֽ�  |          
 * ��|�ļ�ƫ����   | �ļ��� | �ļ���Ŀ���� | Դ�ļ�����   | �ļ�������   | �ļ�ѹ����־λ| �ļ�У����   | �ļ�ʱ����Ϣ | ��չ��Ϣ|              
 *   |----------------------------------------------------------------------------------------------------------------------------
 *   |sizeof(int64)| ������ |                 ......                                                                             |
 *   |�ļ�ƫ����   | �ļ��� |                 ......                                                                             |
 *   |----------------------------------------------------------------------------------------------------------------------------
 * ĩ|  sizeof(int8)      | sizeof(int64)| sizeof(int64)  | sizeof(int64)  | sizeof(int8) |
 * β|  �ļ�����ѹ����־λ| �ļ��������� | �ļ�����ƫ���� | pzbӵ�е��ļ���| end_pzb��־λ|
 *    -------------------------------------------------------------------------------------
 *   ĩβ��ָ���ļ�����������
 */
 
/* pzbĩβ���ڴ���ļ���Ϣ������������±� */
enum {
	FILE_SIZE,           /* �ļ���Ϣ+���ݵ��ֽ��� */                 
	FILE_DATA_SIZE,      /* �ļ����ݵ��ֽ��� */
	FILE_NAME_SIZE,      /* �ļ������ֽ��� */
	FILE_COMPRESSION,    /* �ļ��Ƿ�ѹ�� */
	FILE_CRC,            /* �ļ���CRC */
	FILE_TIME,           /* �ļ�����ʱ�� */
	FILE_EXTEND_INFO,    /* �ļ���չ��Ϣ */
	FILE_OFFSET,         /* �ļ�����ƫ�� */
	FILE_NAME,           /* �ļ��� */
	FILE_COUNT,			 /* ������ */
};

/* pzb����ͷ��䣬�����ж�pzb�Ƿ����� */
#define  PZB_MASK        -1             /* 0xFFFFFFFF */
#define  PZB_MASK_SIZE   8              /* 0xFFFFFFFF */

#define  FLOAT_ZERO      0.00001        /* �����ж�FLOAT�Ƿ�Ϊ0 */

/* �ļ���������������Ҫ���ֽ��� */
#define FILE_SIZE_INDEX_LEN             sizeof(int64)                  /* �ļ�����֮�ļ������ֽ� */
#define FILE_DATA_SIZE_INDEX_LEN        sizeof(int64)                  /* �ļ�����֮�ļ������ֽ� */
#define FILE_NAME_SIZE_INDEX_LEN        sizeof(int16)                  /* �ļ�����֮�ļ����ֽ� */
#define FILE_COMPRESSION_INDEX_LEN      sizeof(int8)                   /* �ļ�����֮ѹ����־λ�ֽ� */
#define FILE_CRC_INDEX_LEN              sizeof(int32)                  /* �ļ�����֮CRC�ֽ� */
#define FILE_TIME_INDEX_LEN             sizeof(int64)                  /* �ļ�����֮�洢ʱ���ֽ� */
#define FILE_OFFSET_INDEX_LEN           sizeof(int64)                  /* �ļ�����֮ƫ�����洢�ֽ� */

#define INDEX_NONAME			(FILE_SIZE_INDEX_LEN\
								+ FILE_DATA_SIZE_INDEX_LEN\
								+ FILE_NAME_SIZE_INDEX_LEN\
								+ FILE_COMPRESSION_INDEX_LEN\
								+ FILE_CRC_INDEX_LEN\
								+ FILE_TIME_INDEX_LEN\
								+ FILE_OFFSET_INDEX_LEN\
								+ EXTEND_INFO_NUM)                                 /* �ļ����������ȣ������ļ��� */

#define INDEX_NONAME_NOOFFSET	(INDEX_NONAME - FILE_OFFSET_INDEX_LEN)             /* �ļ����������ȣ������ļ�����ƫ���� */
#define PZB_SIZE				(40 * 1024 * 1024)                                 /* pzb���ݵĴ�С���� */
#define PZB_FILE_COUNT			100                                                /* pzb�������ļ��������� */
#define PZB_MERGE_SCALE			0.5                                                /* pzb��Ч���ݵ��ڸ÷�ֵ�ͺϲ� */
#define COMPRESS_RATIO			1                                                  /* ����ѹ�����ȱ��� */
#define END_SIZE				26                                                 /* pzbβ���ļ������������Ĵ�С */
#define CACHE_PZB_NUM           1                                                  /* PZB������� */

typedef struct 
{
	PiBool is_compress;      /* �����Ƿ�ѹ�� */
	int8 is_end;             /* �Ƿ���end_pzb */
	int64 size;              /* �����Ĵ�С */
	int64 offset;            /* ������ƫ�� */
	int64 count;             /* pzbӵ���ļ����� */
}IndexInfo;

typedef struct
{
	int64 is_merge;                 /* �жϸ�pzb�Ƿ�ϲ� */
	int64 offset;				    /* �����ļ���������ʼλ�ã���FFFFFFFFFFFFFFFFΪ��ʶ��*/	
	int64 total_size;				/* ����ܴ�С */
	int64 usefull_size;				/* ������ò��ֵĴ�С */
	int64 index;              		/* ��ı�� */
	int32 total_file_count;			/* �����ļ����ܸ��� */
	float proportion;				/* ���õĴ�Сռ�ܴ�С�ı��� */
	int32 usefull_file_count;		/* ���������ļ��ĸ��� */
	int64 major_version;            /* �������ֶ��������汾�ţ����ڱ�֤pzb��˳�� */
	int64 minor_version;            /* �������ֶ����Ĵΰ汾�ţ����ڱ�֤pzb��˳�� */
	wchar pzb_path[NAME_LENGTH];    /* ������֣���ɸ�ʽ�����汾��.�ΰ汾�� */
	void *file_handle;				/* ���д�ļ�ָ�룬һ��Ϊ�գ�ֻ�����һ�鱣�� */
	sint spin_lock;                 /* �����������ڶ��ļ����»���ʱ */
	sint read_count;                /* ��pzb����ȡ�Ĵ��� */
}PZB;

typedef struct
{
	char file_name[NAME_LENGTH];	/* �ļ��� */
	PZB *pzb;						/* �ļ�����Ӧ��pzb���ָ�� */
	float index;					/* �ļ����ڵĿ�ı�� */
	int64 offset;					/* �ļ��ڿ��е�ƫ��λ�� */
	int64 file_size;				/* Դ�ļ��ĳ��� */
	int64 length;					/* pzb��ÿ���ļ���Ŀ�ĳ��� */	
	int64 index_offset;				/* �ļ����������ڵ�����ƫ�� */
	int name_length;				/* �ļ������� */
	int crc;						/* �ļ���У���� */
	int64 time;						/* �ļ���ʱ����Ϣ */	
	int8 compression;				/* �ļ���ѹ����Ϣ */
	int8 encryption;				/* �ļ�������Ϣ */
	PiExtendInfo extend_info;		/* ��չ��Ϣ�ֶ� */
}FileInfo;

typedef struct 
{
	int pzb_count;   				/* pz������pzb��ĸ��� */
	PiBool read_only;               /* �Ƿ�Ϊֻ��Ŀ¼ */
	wchar path[NAME_LENGTH];     	/* pzĿ¼����������ǿɶ�д��Ŀ¼ */
	PiVector pzb_vector;			/* ���б���һ��pzb������Ϊend_pzbʱ��������pzb_vector�� */ 
	PiDhash file_table;				/* �ļ��б� */
	PZB *end_pzb;					/* ���һ�飬Ψһ��д������pzb */
	PiMutex* lock;					/* �ϲ���ɾ����PZ��ʼ������ */
	sint rlock;						/* �����������wlock���ɶ�д�����ص���file_table */
	sint wlock;						/* д�� */
	sint synchronized;              /* ͬ���������ڶ��̶߳�ȡpz�ļ����ᵼ���ļ���ȡʧ��*/
	PZB *cache_pzb[CACHE_PZB_NUM];  /* pzb�������� */
	PiBytes bytes[FILE_COUNT];	    /* д�ļ�ʱ��Ļ����ֽ����飬ÿ�������Ӧ�ļ���һ����Ϣ */
}PZ;

typedef struct  
{
	PiVector pzblist;			    /* ���ϲ���pzb */
	PiVector filelist;              /* ���ϲ����ļ� */
}MergeData;

#endif /* __PZ_TYPE_H__ */
