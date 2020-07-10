
#ifndef __COMPRESS_H__
#define __COMPRESS_H__

#include "pi_lib.h"

//压缩方法类型
typedef enum
{
	LZMA,
	LZF,
	ZLIB,
	DEFLATE,
} CompressMethod;

//清理函数返回值类型
typedef enum
{
	OK,                         /*执行成功，不过还有数据需要清理，还需要继续调用end函数*/
	STREAM_END,                 /*清理结束*/
	RUN_ERROR,                  /*清理出错*/
} compress_ret;


typedef struct
{
	char * next_in;   /* 将要处理的下一个输入 */
	uint avail_in;   /* next_in中还有多少字节是可用的 */
	uint total_in;    /* 输入的总字节数 */
	char * next_out;   /* 将要处理的下一个输出 */
	uint avail_out;   /* next_out中还有多少字节是可用的 */
	uint total_out;   /* 输出的总字节数 */
	CompressMethod method;   /* 压缩方法 */
	void * strm;       /* 实现层使用的数据结构 */
} CompressContext;

#endif /* __COMPRESS_H__ */
