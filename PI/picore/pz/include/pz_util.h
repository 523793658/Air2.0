#ifndef __PZ_UTIL_H__
#define __PZ_UTIL_H__

#include "pz_type.h"

PI_BEGIN_DECLS

/* 路径名拼接，返回路径指针，需要调用处释放 */
wchar* connect_path(wchar *pz_path, wchar* file_name);

/** 
 * 获得pzb的主版本号，次版本号
 * str   ： pzb的名字
 * marjor： 主版本号
 * minor ： 次版本号
 */
PiBool get_pzb_version(wchar *str, int64 *marjor, int64 *minor);

/** 
 * 根据主版本号，次版本号返回pzb名字，需调用处释放
 * marjor： 主版本号
 * minor ： 次版本号
 */
wchar* set_pzb_name(int64 marjor, int64 minor);

/**
 * 从字节缓冲取出数据写入到file中
 */
PiBool init_fileinfo_from_bytes(PiBytes* bytes, FileInfo *file, PiBool is_end);

/** 
 * 读取merge.log包含的pzb->index，删除合并完的pzb
 */
void delete_merged_pzb(PZ *impl, PiBytes *bytes);

/* 对pz包中的每个pzb块初始化 */
void init_contain_pzb(PZ *pz);

/**
 * 将file添加到pz包的file_table中
 * 更新file所在pzb的有效文件个数和块有效大小
 */
void update_filelist_hash(PZB *pzb, PiDhash *file_table, FileInfo *file, FileInfo *file_old);

PI_END_DECLS

#endif /* __PZ_UTIL_H__ */