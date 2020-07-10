#include "pi_lib/pi_vfs_pz.h"

#include "pz_type.h"
#include "pz_util.h"
#include "pz_merge.h"
#include <pi_compress.h>

typedef struct
{
	PiVector *items;
	uint index;
	uint size;
} PiPZDir;

typedef struct
{
	char *dir_name;
	PiPZDir *dir;
} SearchDir;

static PiSelectR	PI_API _forearch_dhash(void* user_data, void* v)
{
	PI_USE_PARAM(user_data);
	return SELECT_NEXT;
}

static PiSelectR	PI_API _forearch_vector(void* user_data, void* v)
{
	PZB *pzb = (PZB *)v;
	PI_USE_PARAM(user_data);
	if(pzb->file_handle != NULL)
	{
		pi_file_close(pzb->file_handle);
	}

	if(pzb != NULL)
	{
		pi_free(pzb);
	}
	
	return SELECT_NEXT;
}

static uint PI_API _hash_func (const void *key)
{
	FileInfo *file_list = (FileInfo *)key;
	char *file_name = file_list->file_name;
	return pi_str_hash(file_name);
}

static PiBool PI_API _equal_func (const void *a, const void *b)
{
	FileInfo *file_list1 = (FileInfo *)a;
	FileInfo *file_list2 = (FileInfo *)b;
	char *a1 = file_list1->file_name;
	char *b1 = file_list2->file_name;
	return pi_str_equal (a1, b1, FALSE);
}

static void _free_pzb(PZB *pzb)
{
	if(NULL != pzb->file_handle)
	{
		pi_file_close(pzb->file_handle);
	}
	pi_free(pzb);
}

/* 索引添加，8扩展不定长长度，8源不定长长度，2名长，1压缩信息 */
static int64 _fill_end_pzb_index(PiBytes *pz_bytes, PiBytes *data_bytes, int64 offset, FileInfo *file, wchar *file_name, void *compress_data, int64 compress_size, int64 len,  int32 crc, PiBool compression, PiFileInfo *file_info, int64 *p_len)
{
	char *file_name_str = pi_wstr_to_str(file_name, PI_CP_UTF8);
	char *temp_data = NULL, *compress_index_data = NULL;
	uint32 index_size = 0, file_name_len = pi_strlen(file_name_str);
	int32 i;
	int64 temp_size = 0;
	PiBytes bytes_temp;
	sint pz_bytes_len;
	uint64 compress_index_size;
	
	pi_bytes_init(&bytes_temp);
	if (file_name)
	{
		/* 将file需要的信息通过多次写入 */
		temp_size = compress_size + INDEX_NONAME_NOOFFSET + file_name_len;
		file->length = temp_size;
		pi_bytes_write_int64(data_bytes, temp_size);
		pi_bytes_write_variable_int(&pz_bytes[FILE_SIZE], temp_size);

		file->file_size = len;
		pi_bytes_write_int64(data_bytes, len);
		pi_bytes_write_variable_int(&pz_bytes[FILE_DATA_SIZE], len);

		file->name_length = file_name_len;
		pi_bytes_write_int16(data_bytes, (int16)file_name_len);
		pi_bytes_write_int16(&pz_bytes[FILE_NAME_SIZE], (int16)file_name_len);

		file->compression = (int8)compression;
		pi_bytes_write_int8(data_bytes, (int8)compression);
		pi_bytes_write_int8(&pz_bytes[FILE_COMPRESSION], (int8)compression);

		file->crc = crc;
		pi_bytes_write_int32(data_bytes, crc);
		pi_bytes_write_int32(&pz_bytes[FILE_CRC], crc);

		file->time = file_info->time_write;
		pi_bytes_write_int64(data_bytes, file->time);
		pi_bytes_write_int64(&pz_bytes[FILE_TIME], file->time);

		pi_bytes_write_data(data_bytes, file_info->extend_info.info, EXTEND_INFO_NUM);
		pi_memcpy(file->extend_info.info, file_info->extend_info.info, EXTEND_INFO_NUM);
		pi_bytes_write_data(&pz_bytes[FILE_EXTEND_INFO], file->extend_info.info, EXTEND_INFO_NUM);

		file->offset = offset;
		pi_bytes_write_variable_int(&pz_bytes[FILE_OFFSET], offset);

		pi_strcpy(file->file_name, file_name_str, file->name_length);

		pi_bytes_write_data (data_bytes, file_name_str, file_name_len);
		pi_bytes_write_data(&pz_bytes[FILE_NAME], file_name_str, file_name_len);

		if(compress_data)
		{
			pi_bytes_write_data(data_bytes, compress_data, (uint)compress_size);
		}
	}
	pi_bytes_write_int64(data_bytes, PZB_MASK);
	pi_free(file_name_str);

	for (i = 0; i < FILE_COUNT; ++i)
	{
		pi_bytes_rindex (&pz_bytes[i], 0);
		pz_bytes_len = pi_bytes_size(&pz_bytes[i]);
		index_size += pz_bytes_len;
		pi_bytes_read_data (&pz_bytes[i], &temp_data, pz_bytes_len);
		pi_bytes_rindex (&pz_bytes[i], 0);
		pi_bytes_write_data(&bytes_temp, temp_data, pz_bytes_len);
	}
	/* index_size是索引数据长度，pz_bytes[i]是索引信息，compress_data只写到data_bytes中了 */
	pi_bytes_read_data(&bytes_temp, &temp_data, index_size);
	compress_index_data = (char *)pi_malloc0(index_size);
	compress_index_size = pi_compress(temp_data, index_size, compress_index_data, index_size, LZF);

	if (compress_index_size == 0)
	{
		compress_index_size = index_size;
		pi_bytes_write_data(data_bytes, temp_data, index_size);
		pi_bytes_write_int8(data_bytes, 0);
	}
	else
	{
		pi_bytes_write_data(data_bytes, compress_index_data, (uint)compress_index_size);
		pi_bytes_write_int8(data_bytes, 1);
	}
	pi_bytes_clear(&bytes_temp, TRUE);
	pi_bytes_write_int64(data_bytes, index_size);
	pi_free(compress_index_data);
	if(file_name)
	{
		temp_size = compress_size + INDEX_NONAME_NOOFFSET + file_name_len;
		*p_len = *p_len + temp_size + compress_index_size + PZB_MASK_SIZE;
	}
	return (offset + temp_size);
}

/* 获得文件信息，将获得模块对应目录的文件信息，返回是否成功 */
static PiBool PI_API _get_info(void* mod_data, const wchar* file, PiFileInfo* info)
{
	PZ *pz = (PZ *)mod_data;
	char *name_tmp = NULL;
	FileInfo file_list;
	FileInfo *e = NULL;
	pi_memset(&file_list, 0, sizeof(FileInfo));
	if(!pz)
	{
		pi_error_set(ERROR_TYPE_PARAMATER_NULL, 0, NULL, __FILE__, __LINE__);
		return FALSE;
	}
	if(!file)
	{
		pi_error_set(ERROR_TYPE_PARAMATER_NULL, 0, NULL, __FILE__, __LINE__);
		return FALSE;
	}
	if(!info)
	{
		pi_error_set(ERROR_TYPE_PARAMATER_NULL, 0, NULL, __FILE__, __LINE__);
		return FALSE;
	}

	name_tmp = pi_wstr_to_str(file, PI_CP_UTF8);
	pi_strcpy(file_list.file_name, name_tmp, pi_strlen(name_tmp));
	pi_free(name_tmp);
	pi_lock_read_lock(&pz->wlock, &pz->rlock, LOCK_COUNT);

	/* 检查是否有该文件 */
	if(!pi_dhash_lookup(&pz->file_table, &file_list, &e))
	{
		/* 指定的文件不存在 */
		pi_lock_read_free(&pz->rlock);
		pi_error_set(ERROR_TYPE_TARGET_NOT_FOUND, 0, NULL, __FILE__, __LINE__);
		return FALSE;
	}
	info->crc = e->crc;
	info->time_create = 0;
	info->time_write = 0;
	info->size = e->file_size;
	pi_memcpy(info->extend_info.info, e->extend_info.info, EXTEND_INFO_NUM);
	pi_lock_read_free(&pz->rlock);
	return TRUE;
}

/* 获得文件或目录的大小，返回-1表示失败，失败可查错误编号 */
static int64		PI_API _get_size (void* mod_data, const wchar* file)
{
	PZ* pz = (PZ*)mod_data;
	FileInfo file_list;
	FileInfo* e = NULL;
	int64 size;
	char *name_tmp = NULL;

	if(!pz)
	{
		pi_error_set(ERROR_TYPE_PARAMATER_NULL, 0, NULL, __FILE__, __LINE__);
		return -1;
	}
	if(!file)
	{
		pi_error_set(ERROR_TYPE_PARAMATER_NULL, 0, NULL, __FILE__, __LINE__);
		return -1;
	}

	name_tmp = pi_wstr_to_str(file, PI_CP_UTF8);
	pi_strcpy(file_list.file_name, name_tmp, pi_strlen(name_tmp));
	pi_free(name_tmp);
	pi_lock_read_lock(&pz->wlock, &pz->rlock, LOCK_COUNT);

	/* 检查是否有该文件 */
	if(!pi_dhash_lookup(&pz->file_table, &file_list, &e))
	{
		/* 指定的文件不存在 */
		pi_lock_read_free(&pz->rlock);
		pi_error_set(ERROR_TYPE_TARGET_NOT_FOUND, 0, NULL, __FILE__, __LINE__);
		return -1;
	}
	size = e->file_size;
	pi_lock_read_free(&pz->rlock);
	return size;
}

/* 设置文件时间，禁止使用 */
static PiBool		PI_API _set_time (void* mod_data, const wchar* file, int64 create_time, int64 write_time)
{
	pi_error_set(ERROR_TYPE_NOT_SUPPORTED, 0, NULL, __FILE__, __LINE__);
	return FALSE;	
}

/* 文件重命名， 禁止使用 */
static PiBool	PI_API _rename (void* mod_data, const wchar* old_name, const wchar* new_name)
{
	pi_error_set(ERROR_TYPE_NOT_SUPPORTED, 0, NULL, __FILE__, __LINE__);
	return FALSE;
}

/* 创建目录，禁止使用 */
static PiBool		PI_API _dir_create (void* mod_data, const wchar* dir)
{
	pi_error_set(ERROR_TYPE_NOT_SUPPORTED, 0, NULL, __FILE__, __LINE__);
	return FALSE;
}

/* 删除空目录，禁止使用 */
static PiBool		PI_API _dir_delete (void* mod_data, const wchar* dir)
{
	pi_error_set(ERROR_TYPE_NOT_SUPPORTED, 0, NULL, __FILE__, __LINE__);
	return FALSE;
}

/* 目录是否为空，禁止使用 */
static PiBool	PI_API _dir_is_empty(void* mod_data, const wchar* dir, PiBool *is_empty)
{
	pi_error_set(ERROR_TYPE_NOT_SUPPORTED, 0, NULL, __FILE__, __LINE__);
	return FALSE;
}

/* 判断item_name是否为目录，是返回true否则返回false */
static PiBool _is_dir_item(const char *dir_name, const char *item_name)
{
	uint i = 0;
	if (!dir_name[0])
	{
		return TRUE;
	}
	while (1)
	{
		char c = dir_name[i];
		if (!c)
		{
			if (item_name[i] == '/')
			{
				return TRUE;
			}
			return FALSE;
		}
		if (item_name[i] != c)
		{
			return FALSE;
		}
		i++;
	}
}

static PiSelectR PI_API _search_dir(SearchDir *search_data, FileInfo *item)
{
	if (_is_dir_item(search_data->dir_name, item->file_name))
	{
		pi_vector_push(search_data->dir->items, item);
		search_data->dir->size++;
	}
	return SELECT_NEXT;
}

static PiCompR PI_API _item_name_comp(void *user_data, const FileInfo *a, const FileInfo *b)
{
	return pi_str_compare(a->file_name, b->file_name);
}

/* 打开目录 */
static PiPZDir *PI_API _dir_open(void *mod_data, const wchar *dir_name)
{
	SearchDir search_data;
	PZ *pz = (PZ *)mod_data;
	PiPZDir *dir = pi_new0(PiPZDir, 1);

	dir->items = pi_vector_new();

	search_data.dir_name = pi_wstr_to_str(dir_name, PI_CP_UTF8);
	search_data.dir = dir;

	pi_dhash_foreach(&pz->file_table, _search_dir, &search_data);

	pi_vector_sort(dir->items, _item_name_comp, NULL);

	pi_free(search_data.dir_name);

	return dir;
}

/* 读取目录项 */
static PiBool PI_API _dir_read(void *mod_data, PiPZDir *dir, PiFileNameInfo *info)
{
	if (dir->index < dir->size)
	{
		FileInfo *item = pi_vector_get(dir->items, dir->index);

		wchar *name = pi_str_to_wstr(item->file_name, PI_CP_UTF8);
		pi_wstrcpy(info->name, name, pi_wstrlen(name));
		pi_free(name);

		info->info.type = FILE_TYPE_REGULAR;
		info->info.time_create = item->time;
		info->info.time_write = item->time;
		info->info.size = item->file_size;
		info->info.crc = item->crc;

		pi_memcpy(&info->info.extend_info, &item->extend_info, sizeof(PiExtendInfo));

		dir->index++;
		return TRUE;
	}

	return FALSE;
}

/* 关闭目录 */
static PiBool PI_API _dir_close(void *mod_data, PiPZDir *dir)
{
	pi_vector_free(dir->items);
	pi_free(dir);
	return TRUE;
}

/* 取得文件的crc值，返回是否成功，如果成功则crc参数放置crc值，失败可查错误编号 */
static PiBool		PI_API _file_get_crc32 (void* mod_data, const wchar* file, uint32 *crc)
{
	PZ* pz = (PZ*)mod_data;
	FileInfo file_list;
	FileInfo* e = NULL;
	char *name_tmp = NULL;

	if(!pz)
	{
		pi_error_set(ERROR_TYPE_PARAMATER_NULL, 0, NULL, __FILE__, __LINE__);
		return -1;
	}
	if(!file)
	{
		pi_error_set(ERROR_TYPE_PARAMATER_NULL, 0, NULL, __FILE__, __LINE__);
		return -1;
	}

	name_tmp = pi_wstr_to_str(file, PI_CP_UTF8);
	pi_strcpy(file_list.file_name, name_tmp, pi_strlen(name_tmp));
	pi_free(name_tmp);
	pi_lock_read_lock(&pz->wlock, &pz->rlock, LOCK_COUNT);

	/* 检查是否有该文件 */
	if(!pi_dhash_lookup(&pz->file_table, &file_list, &e))
	{
		/* 指定的文件不存在 */
		pi_lock_read_free(&pz->rlock);
		pi_error_set(ERROR_TYPE_TARGET_NOT_FOUND, 0, NULL, __FILE__, __LINE__);
		return -1;
	}
	*crc = e->crc;
	pi_lock_read_free(&pz->rlock);
	return TRUE;
}

/* 打开文件，返回文件指针，NULL为失败，失败可查错误编号 */
static void*		PI_API _file_open (void* mod_data, const wchar* file, PiFileOpenMode mode)
{
	PZ* pz = (PZ*)mod_data;
	wchar *file_name = (wchar *)pi_malloc0(sizeof(wchar) * (pi_wstrlen(file) + 1));
	pi_memcpy(file_name, file, pi_wstrlen(file) * sizeof(wchar));
	if(!pz)
	{
		pi_error_set(ERROR_TYPE_PARAMATER_NULL, 0, NULL, __FILE__, __LINE__);
		return NULL;
	}
	if(!file)
	{
		pi_error_set(ERROR_TYPE_PARAMATER_NULL, 0, NULL, __FILE__, __LINE__);
		return NULL;
	}
	if(!((mode & FILE_OPEN_READ) || (mode & FILE_OPEN_WRITE)))
	{
		pi_error_set(ERROR_TYPE_PARAMATER_ILLEGAL, 0, NULL, __FILE__, __LINE__);
		return NULL;
	}
	return (void*)file_name;
}

static PiBool		PI_API _file_close (void* mod_data, const void* handle)
{
	PZ *pz = (PZ *)mod_data;
	if(!pz)
	{
		pi_error_set(ERROR_TYPE_PARAMATER_NULL, 0, NULL, __FILE__, __LINE__);
		return FALSE;
	}
	if(!handle)
	{
		pi_error_set(ERROR_TYPE_PARAMATER_NULL, 0, NULL, __FILE__, __LINE__);
		return FALSE;
	}
	pi_free((void*)handle);
	return TRUE;
}

static PiBool		PI_API _file_size (void* mod_data, const void* handle, int64* size)
{
	wchar *file = (wchar *)handle;
	*size = _get_size (mod_data, file);
	if (*size <= 0)
	{
		return FALSE;
	}
	return TRUE;
}

static PiBool		PI_API _file_crc32 (void* mod_data, const void* handle, uint32 *crc)
{
	wchar *file = (wchar *)handle;
	return _file_get_crc32 (mod_data, file, crc);
}

/* 读取文件，成功则返回读到的长度，没有找到文件返回0，其他异常返回-1 */
static int64   PI_API _pz_read(char *data, int64 len, int64 offset, void * pz, const wchar *file_name, PiBool from_end)
{
	PZB *file_pzb = NULL;
	PZ *impl = (PZ *)pz;
	FileInfo *file = NULL;
	int64 content_len, content_offset, read_size;
	void *file_handle;
	FileInfo el;
	char *compress_data = NULL, *file_data = NULL, *name_tmp = NULL;
	PiBool is_compress = FALSE;

	name_tmp = pi_wstr_to_str(file_name, PI_CP_UTF8);
	pi_strcpy(el.file_name, name_tmp, pi_strlen(name_tmp));
	pi_free(name_tmp);
	
	/* 用读写锁实现对hash表的访问，该效率较低，应该对hash表的每项分别用读写锁 */
	pi_lock_read_lock(&impl->wlock, &impl->rlock, LOCK_COUNT);
	/* 查找是否有该file，有则存入file */
	pi_dhash_lookup(&impl->file_table, &el, &file);
	pi_lock_read_free(&impl->rlock);

	if (file == NULL)
	{
		return 0;
	}

	is_compress = (PiBool)file->compression;
	content_len = file->length - INDEX_NONAME_NOOFFSET - file->name_length;
	content_offset = file->offset + INDEX_NONAME_NOOFFSET + file->name_length;
	
	file_pzb = file->pzb;
	if(content_len <= 0)
	{
		return -1;
	}

	pi_lock_synchronized_lock(&impl->synchronized, LOCK_COUNT);
	file_handle = file_pzb->file_handle;
	if(file_handle == NULL)
	{
		file_handle = pi_file_open(file_pzb->pzb_path, FILE_OPEN_READ);
		if (file_handle == NULL)
		{
			pi_lock_synchronized_free(&impl->rlock);
			return -1;
		}
	}

	/* todo:效率不够时可以将普通读改为内存映射文件方式读 */

	compress_data = (char *)pi_malloc0((uint)content_len);
	read_size = pi_file_read(file_handle, content_offset, FALSE, compress_data, (uint)content_len);	

	/* 缓冲策略，保存访问次数最多的pzb句柄 */
	pi_lock_spin_lock(&file_pzb->spin_lock, LOCK_COUNT);
	file_pzb->file_handle = file_handle;
	++file_pzb->read_count;

	if(*impl->cache_pzb == NULL)
	{
		*impl->cache_pzb = file_pzb;
	}
	else if(file_pzb->read_count < (*impl->cache_pzb)->read_count)
	{
		pi_file_close(file_pzb->file_handle);
		file_pzb->file_handle = NULL;
	}
	else if (*impl->cache_pzb != file_pzb)
	{
		pi_file_close((*impl->cache_pzb)->file_handle);
		(*impl->cache_pzb)->file_handle = NULL;
		*impl->cache_pzb = file_pzb;
	}
	pi_lock_spin_free(&file_pzb->spin_lock);
	pi_lock_synchronized_free(&impl->synchronized);

	if(read_size <= 0)
	{
		return -1;
	}
	if (is_compress)
	{	
		file_data = (char *)pi_malloc0((uint)file->file_size);
		read_size = pi_uncompress(compress_data, (uint32)content_len, file_data, (uint32)file->file_size, LZF);

		if (!from_end)
		{
			pi_memcpy(data, file_data, (uint)len);
		}
		else
		{
			if(offset < len)
			{
				return -1;
			}
			pi_memcpy(data, file_data + (file->file_size - offset), (uint)len);
		}

		read_size = len;
		pi_free(file_data);
	}
	else
	{
		if(read_size < len) 
		{
			len = read_size;
		}
		if(!from_end)
		{
			pi_memcpy(data, compress_data, (uint)len);
		}
		else
		{
			pi_memcpy(data, compress_data + (file->file_size - offset), (uint)len);
			read_size = len;
		}
	}
	pi_free(compress_data);
	return read_size;
}

/**
 * 文件读取，数据读入data中。
 * 成功返回读取到的数据长度，失败则返回false
 */
static sint			PI_API _file_read (void* mod_data, const void* handle, int64 offset, PiBool from_end, char* data, uint len)
{
	PZ *pz = (PZ *)mod_data;
	wchar *file_name = (wchar *)handle;
	if(!pz)
	{
		pi_error_set(ERROR_TYPE_PARAMATER_NULL, 0, NULL, __FILE__, __LINE__);
		return FALSE;
	}
	if(!handle)
	{
		pi_error_set(ERROR_TYPE_PARAMATER_NULL, 0, NULL, __FILE__, __LINE__);
		return FALSE;
	}

	return (sint)_pz_read(data, len, offset, mod_data, file_name, from_end);
}

/* data：写入的数据
 * mod_data: pz的指针
 * handle: 被写入文件的句柄
 * file_info: 被写入文件的info
 * len: 写入data的长度
 * 同时文件与pzb、pz联系起来
 */
static PiBool		PI_API _file_write_ext (void* mod_data, const void* handle,  PiFileInfo *file_info, char* data, uint len)
{
	PZ *impl = (PZ *)mod_data;
	uint pzb_index;
	int8 is_end = 0;
	int32 i, crc = pi_get_crc32(data, len);
	int64 temp_len, offset, temp_data_size, compress_data_len;
	char *temp_file_data = NULL, *compress_data = NULL;
	void *file_handle;
	wchar *file_name = (wchar *)handle;
	wchar *pzb_name = NULL, *pzb_path = NULL;
	PiBytes *data_bytes = pi_bytes_new();
	FileInfo file, file_old;
	PiBool is_compress = FALSE, write_err = FALSE;
	pi_memset(&file, 0, sizeof(FileInfo));

	if (file_name == NULL)
	{
		pi_bytes_free(data_bytes);
		return FALSE;
	}

	/** 
	 * pz没有握住end_pzb文件时：1.没有end_pzb 2.之前操作的end_pzb已经不再作为end_pzb
     * 否则写数据操作end_pzb
	 */
	if (impl->end_pzb == NULL)
	{
		PZB *pzb_instance = (PZB *)pi_vector_peek(&impl->pzb_vector);
		/* pzb_instance为Null说明没有创建过pzb，应该创建第一个pzb */
		if (pzb_instance == NULL)
		{
			pzb_index = 0;
		}
		else
	    {
			pzb_index = (uint)(pzb_instance->major_version + 1);
		}

		impl->end_pzb = (PZB *)pi_malloc0(sizeof(PZB));
		impl->end_pzb->major_version = pzb_index;
		pzb_name = pi_wstr_from_int(pzb_index);  /* 写的时候新建的pzb全是整数 */
		pzb_path = connect_path(impl->path, pzb_name);
		pi_wstrcpy (impl->end_pzb->pzb_path, pzb_path, pi_wstrlen(pzb_path));
		++impl->pzb_count;
		pi_free(pzb_name);
		pi_free(pzb_path);
	}

	if (impl->end_pzb->file_handle == NULL)
	{		
		/* 打开end_pzb获得其句柄，一直握住直到end_pzb不能在作为end_pzb时释放 */
		file_handle = pi_file_open(impl->end_pzb->pzb_path, FILE_OPEN_WRITE | FILE_OPEN_READ);
		impl->end_pzb->file_handle = file_handle;
	}

	/**
	 * COMPRESS_RATIO为1意思是压缩后的长度不可能比未压缩时的长度更长
	 * pi_compress 返回0意味着压缩失败
	 */
	file.offset = impl->end_pzb->offset;
	file.index = (float)impl->end_pzb->index;
	compress_data_len = len / COMPRESS_RATIO;
	compress_data = (char *)pi_malloc0((uint)(compress_data_len + 1));
	compress_data_len = pi_compress(data, (uint32)len, compress_data, (uint32)compress_data_len, LZF);

	if (compress_data_len == 0)
	{
		compress_data_len = len;
		is_compress = FALSE;
		offset = _fill_end_pzb_index(impl->bytes, data_bytes, impl->end_pzb->offset, &file, file_name, data, compress_data_len, len, crc, is_compress, file_info, &temp_len);
	}
	else
	{
		is_compress = TRUE;
		offset = _fill_end_pzb_index(impl->bytes, data_bytes, impl->end_pzb->offset, &file, file_name, compress_data, compress_data_len, len, crc, is_compress, file_info, &temp_len);
	}
	pi_free(compress_data);

	pi_bytes_write_int64(data_bytes, offset);
	pi_bytes_write_int64(data_bytes, impl->end_pzb->total_file_count + 1);
	file.pzb = impl->end_pzb;
	/* 判断现在这个pzb是否作为end_pzb，如果end_pzb的大小超过40M或者文件数超过了100则不再将其作为end_pzb */
	if (impl->end_pzb->total_file_count >= PZB_FILE_COUNT - 1 || impl->end_pzb->offset + temp_len + END_SIZE >= PZB_SIZE)
	{
		is_end = 0;
	}
	else
	{
		is_end = 1;
	}
	pi_bytes_write_int8(data_bytes, is_end);

	pi_bytes_rindex (data_bytes, 0);
	temp_data_size = pi_bytes_size(data_bytes);	
	pi_bytes_read_data(data_bytes, &temp_file_data, (int)temp_data_size);
	/* 此处可以用内存映射文件方式实现 */

	/* 暂时没有使用内存映射 */
	file_handle = impl->end_pzb->file_handle;
	write_err = pi_file_write(file_handle, impl->end_pzb->offset, FALSE, temp_file_data, (uint)temp_data_size);
	if (!write_err)
	{
		pi_bytes_free(data_bytes);
		return FALSE;
	}

	impl->end_pzb->total_size = impl->end_pzb->offset + temp_data_size;
	impl->end_pzb->offset = offset;
	pi_bytes_free(data_bytes);
	pi_lock_read_lock(&impl->wlock, &impl->rlock, LOCK_COUNT);
	pi_atomic_add (&impl->wlock, 1);
	
	update_filelist_hash(impl->end_pzb, &impl->file_table, &file, &file_old);

	pi_atomic_add (&impl->wlock, -1);
	pi_lock_read_free(&impl->rlock);
	/* end_pzb不在作为end_pzb动作，PZ中的end_pzb置为null */
	if (impl->end_pzb->total_file_count >= PZB_FILE_COUNT || impl->end_pzb->total_size >= PZB_SIZE)
	{
		for (i = 0; i < FILE_COUNT; ++i)
		{
			pi_bytes_clear(&impl->bytes[i], TRUE);
		}

		pi_file_close(impl->end_pzb->file_handle);
		impl->end_pzb->file_handle = NULL;
		
		pi_vector_push(&impl->pzb_vector, impl->end_pzb);
		impl->end_pzb = NULL;
	}
	
	return TRUE;
}

/** 
 *删除文件, 向file_table中将文件内容写为空，下次初始化时将其删除
 */
static PiBool		PI_API _file_delete (void* mod_data, const wchar* file)
{
	char *data = NULL;
	uint len = 0;
	PiFileInfo file_info;
	pi_memset(&file_info, 0, sizeof(PiFileInfo));

	return _file_write_ext(mod_data, file, &file_info, data, len);
}

/* 该接口暂时禁止使用 */
static PiBool			PI_API _set_info (void* mod_data, const void* handle, PiExtendInfo *extend_info)
{
	PZ *impl = (PZ *)mod_data;
	wchar *file_name = (wchar *)handle;
	FileInfo file_list;
	FileInfo *e = NULL;
	char *name_tmp = NULL;

	if(!impl)
	{
		pi_error_set(ERROR_TYPE_PARAMATER_NULL, 0, NULL, __FILE__, __LINE__);
		return FALSE;
	}
	if(!handle)
	{
		pi_error_set(ERROR_TYPE_PARAMATER_NULL, 0, NULL, __FILE__, __LINE__);
		return FALSE;
	}
	pi_memset(&file_list, 0, sizeof(FileInfo));
	name_tmp = pi_wstr_to_str(file_name, PI_CP_UTF8);
	pi_strcpy(file_list.file_name, name_tmp, pi_strlen(name_tmp));
	pi_free(name_tmp);

	/* 检查是否有该文件 */
	if(!pi_dhash_lookup(&impl->file_table, &file_list, &e))
	{
		/* 指定的文件不存在 */
		pi_error_set(ERROR_TYPE_TARGET_NOT_FOUND, 0, NULL, __FILE__, __LINE__);
		return FALSE;
	}

	/* 写文件和索引相对应的附加信息 */
	if (e->pzb->file_handle == NULL)
	{
		e->pzb->file_handle = pi_file_open(impl->end_pzb->pzb_path, FILE_OPEN_WRITE | FILE_OPEN_READ);
	}
	pi_file_write(e->pzb->file_handle, e->offset + 10, FALSE, (char *)extend_info->info, 10);
	pi_file_write(e->pzb->file_handle, e->pzb->offset + 8 + e->index_offset + 10, FALSE, (char *)extend_info->info, 10);
	pi_file_close(e->pzb->file_handle);
	e->pzb->file_handle = NULL;

	return TRUE;
}

static PiBool			PI_API  _file_truncate(void* mod_data, const void* handle, int64 size)
{
	pi_error_set(ERROR_TYPE_NOT_SUPPORTED, 0, NULL, __FILE__, __LINE__);
	return FALSE;
}

/* 初始化pz包 */
void*   PI_API init_pz(void *data)
{
	PZ *pz = (PZ *)pi_malloc0(sizeof(PZ));
	wchar *path = (wchar *)data;
	wchar *pz_path = NULL, *path_tmp = NULL;
	int index = 0;
	pz->lock = pi_mutex_new();

	if (path == NULL)
	{
		pi_free(pz);
		return NULL;
	}

	pi_mutex_lock(pz->lock);

	/* 创建pz包中的file_table */
	pi_dhash_init(&pz->file_table, sizeof(FileInfo), 0.6f, _hash_func, _equal_func);
	
	path_tmp = (wchar*)pi_malloc(pi_wstrlen(path) * sizeof(wchar));
	path_tmp = pi_wstrcpy_malloc(path, pi_wstrlen(path));
	index = pi_wstr_char_index(path, L'|');
	do 
	{
		if (index > 0)
		{
			pz_path = pi_wstrcpy_malloc(path_tmp, (uint)index);
			pi_wstrcpy(path_tmp, path_tmp + index + 1, pi_wstrlen(path_tmp) - index - 1);
			index = pi_wstr_char_index(path_tmp, L'|');
			pz->read_only = TRUE;
		}
		else
		{
			pz_path = pi_wstrcpy_malloc(path_tmp, pi_wstrlen(path_tmp));
			pz->read_only = FALSE;
		}

		pi_file_dir_create(pz_path);
		pi_memset(pz->path, 0, NAME_LENGTH);
		pi_memcpy(pz->path, pz_path, pi_wstrlen(pz_path) * sizeof(wchar));

		init_contain_pzb(pz);

		pi_free(pz_path);

	} while (index > 0 || pz->read_only == TRUE);

	pi_free(path_tmp);
	pi_mutex_unlock(pz->lock);
	return pz;
}

/* 模块函数映射，pz初始化 */
PiBool			PI_API mod_open (PiVfsMod* mod, void *data, const wchar *real_path)
{
	wchar *pz_path = (wchar *)real_path;

	mod->get_info = _get_info;
	mod->get_size = _get_size;
	mod->set_time = _set_time;
	mod->rename = _rename;
	mod->dir_create = _dir_create;
	mod->dir_delete = _dir_delete;
	mod->dir_is_empty = _dir_is_empty;
	mod->dir_open = _dir_open;
	mod->dir_read = _dir_read;
	mod->dir_close = _dir_close;
	mod->file_get_crc32 = _file_get_crc32;
	mod->file_delete = _file_delete;
	mod->file_open = _file_open;
	mod->file_close = _file_close;
	mod->file_size = _file_size;
	mod->file_crc32 = _file_crc32;
	mod->file_read = _file_read;
	
	/* 禁止使用该接口 */
	/*mod->file_write = _file_write;*/
	
	mod->file_write_ext = _file_write_ext;
	
	/* 禁止使用该接口 */
	mod->set_extend_info = _set_info;

	mod->file_truncate = _file_truncate;
	mod->collate = pz_merge; 
	mod->thread_count = 0;
	mod->mod_data = init_pz(pz_path);
	if (!mod->mod_data) {
		return FALSE;
	}
	return TRUE;
}

/* 模块释放函数 */
PiBool			PI_API mod_close (void* mod_data)
{
	PZ *impl = (PZ*) mod_data;
	PZB *end_pzb = impl->end_pzb;
	if (end_pzb)
	{
		pi_file_close(end_pzb->file_handle);
		pi_free(end_pzb);
	}
	pi_vector_foreach (&impl->pzb_vector, _forearch_vector, NULL);
	//pi_vector_clear(&impl->pzb_vector, TRUE);
	pi_dhash_foreach(&impl->file_table, _forearch_dhash, NULL);
	//pi_dhash_clear(&impl->file_table, TRUE);
	if (impl->lock)
	{
		pi_mutex_free(impl->lock);
	}
	pi_free(impl);
	return TRUE;
}