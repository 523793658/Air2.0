#ifndef __PZ_MERGE_H__
#define __PZ_MERGE_H__

#include "pz_util.h"

PI_BEGIN_DECLS

/* ��bytes���������Ϣ���ɹ�����0 */
int64 PI_API fill_merge_pzb_index(PiBytes *bytes, int64 write_offset);

/* ����Ҫ�ϲ����ļ�װ��MergeData�µ�filelist */
PiSelectR PI_API foreach_valid_file(void *user_data, void *value);

/* pz���ϲ�����������ɾ���ϲ����pzb */
void PI_API pz_merge(void *pz, int64 time);

PI_END_DECLS

#endif /* __PZ_MERGE_H__ */