
#ifndef __COMPRESS_H__
#define __COMPRESS_H__

#include "pi_lib.h"

//ѹ����������
typedef enum
{
	LZMA,
	LZF,
	ZLIB,
	DEFLATE,
} CompressMethod;

//����������ֵ����
typedef enum
{
	OK,                         /*ִ�гɹ�����������������Ҫ��������Ҫ��������end����*/
	STREAM_END,                 /*�������*/
	RUN_ERROR,                  /*�������*/
} compress_ret;


typedef struct
{
	char * next_in;   /* ��Ҫ�������һ������ */
	uint avail_in;   /* next_in�л��ж����ֽ��ǿ��õ� */
	uint total_in;    /* ��������ֽ��� */
	char * next_out;   /* ��Ҫ�������һ����� */
	uint avail_out;   /* next_out�л��ж����ֽ��ǿ��õ� */
	uint total_out;   /* ��������ֽ��� */
	CompressMethod method;   /* ѹ������ */
	void * strm;       /* ʵ�ֲ�ʹ�õ����ݽṹ */
} CompressContext;

#endif /* __COMPRESS_H__ */
