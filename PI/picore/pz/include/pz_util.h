#ifndef __PZ_UTIL_H__
#define __PZ_UTIL_H__

#include "pz_type.h"

PI_BEGIN_DECLS

/* ·����ƴ�ӣ�����·��ָ�룬��Ҫ���ô��ͷ� */
wchar* connect_path(wchar *pz_path, wchar* file_name);

/** 
 * ���pzb�����汾�ţ��ΰ汾��
 * str   �� pzb������
 * marjor�� ���汾��
 * minor �� �ΰ汾��
 */
PiBool get_pzb_version(wchar *str, int64 *marjor, int64 *minor);

/** 
 * �������汾�ţ��ΰ汾�ŷ���pzb���֣�����ô��ͷ�
 * marjor�� ���汾��
 * minor �� �ΰ汾��
 */
wchar* set_pzb_name(int64 marjor, int64 minor);

/**
 * ���ֽڻ���ȡ������д�뵽file��
 */
PiBool init_fileinfo_from_bytes(PiBytes* bytes, FileInfo *file, PiBool is_end);

/** 
 * ��ȡmerge.log������pzb->index��ɾ���ϲ����pzb
 */
void delete_merged_pzb(PZ *impl, PiBytes *bytes);

/* ��pz���е�ÿ��pzb���ʼ�� */
void init_contain_pzb(PZ *pz);

/**
 * ��file��ӵ�pz����file_table��
 * ����file����pzb����Ч�ļ������Ϳ���Ч��С
 */
void update_filelist_hash(PZB *pzb, PiDhash *file_table, FileInfo *file, FileInfo *file_old);

PI_END_DECLS

#endif /* __PZ_UTIL_H__ */