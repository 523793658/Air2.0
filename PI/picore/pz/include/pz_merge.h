#ifndef __PZ_MERGE_H__
#define __PZ_MERGE_H__

#include "pz_util.h"

PI_BEGIN_DECLS

/* 给bytes添加索引信息，成功返回0 */
int64 PI_API fill_merge_pzb_index(PiBytes *bytes, int64 write_offset);

/* 将需要合并的文件装入MergeData下的filelist */
PiSelectR PI_API foreach_valid_file(void *user_data, void *value);

/* pz包合并整理，并尝试删除合并完的pzb */
void PI_API pz_merge(void *pz, int64 time);

PI_END_DECLS

#endif /* __PZ_MERGE_H__ */