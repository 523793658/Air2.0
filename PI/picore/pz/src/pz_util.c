#include "pz_util.h"

wchar* connect_path(wchar *pz_path, wchar* file_name)
{
	uint32 name_len = pi_wstrlen(file_name);
	uint32 path_len = pi_wstrlen(pz_path);
	uint len = path_len + pi_wstrlen(L"/") + name_len + 1;
	wchar *path = pi_new0(wchar, len);
	pi_wstr_connect_path(path, len, pz_path, file_name);
	return path;
}

PiBool get_pzb_version(wchar *str, int64 *marjor, int64 *minor)
{
	int32 find_index;
	wchar tmp[NAME_LENGTH];
	find_index = pi_wstr_text_index(str, L".");
	if(find_index > 0)
	{
		pi_wstrcpy(tmp, str, find_index);
		pi_wstr_parse_number(tmp, marjor);
		pi_wstrcpy(tmp, str + find_index + 1, pi_wstrlen(str) - find_index);
		pi_wstr_parse_number(tmp, minor);
	}
	else
	{
		pi_wstr_parse_number(str, marjor);
		*minor = 0;
	}
	return TRUE;
}

wchar* set_pzb_name(int64 marjor, int64 minor)
{
	wchar *str;
	uint size = 0;
	wchar *mar = pi_wstr_from_int(marjor);
	wchar *min = pi_wstr_from_int(minor);
	
	size = pi_wstrlen(mar) + pi_wstrlen(L".") + pi_wstrlen(min) + 1;
	str = pi_new0(wchar, size);

	pi_wstr_cat(str, size, mar);
	pi_wstr_cat(str, size, L".");
	pi_wstr_cat(str, size, min);
	pi_free(mar);
	pi_free(min);
	return str;
}

PiBool init_fileinfo_from_bytes(PiBytes* bytes, FileInfo *file, PiBool is_end)
{
	char *temp;
	uint size = pi_bytes_size(bytes);

	file->length = pi_bytes_read_int64_unsafe(bytes);
	if (is_end)
	{
		if (file->length > size)
		{
			return FALSE;
		}
	}

	file->file_size = pi_bytes_read_int64_unsafe(bytes);

	file->name_length = pi_bytes_read_uint16_unsafe (bytes);

	if (!is_end)
	{
		if ((uint32)(file->name_length + INDEX_NONAME) > size)
		{
			return FALSE;
		}
	}
	file->compression = pi_bytes_read_uint8_unsafe (bytes);

	file->crc = pi_bytes_read_uint32_unsafe (bytes);

	file->time = pi_bytes_read_int64_unsafe (bytes);

	pi_bytes_read_data(bytes, &temp, EXTEND_INFO_NUM);
	pi_memcpy(file->extend_info.info, temp, EXTEND_INFO_NUM);

	pi_bytes_read_data (bytes, &temp, file->name_length);
	pi_strcpy(file->file_name, temp, file->name_length);

	return TRUE;
}

void delete_merged_pzb(PZ *impl, PiBytes *bytes)
{
	wchar *pzb_path = NULL;

	while(pi_bytes_read_wstr(bytes, &pzb_path) >= 0)
	{ 
		pi_file_delete(pzb_path);
	}
	pi_bytes_clear(bytes, TRUE);
}

/**
 * 根据pzb的版本号从小到大排序
 */
PiCompR	PI_API _sort_pzb_by_version(void *user_data, const void *a, const void *b)
{
	PZB *pa = (PZB *)a;
	PZB *pb = (PZB *)b;
	if(pa->major_version == pb->major_version)
	{
		// 不可能有名字相同的pzb文件
		return pa->minor_version > pb->minor_version ? PI_COMP_GREAT : PI_COMP_LESS;
	}
	return pa->major_version > pb->major_version ? PI_COMP_GREAT : PI_COMP_LESS;
}

void update_filelist_hash(PZB *pzb, PiDhash *file_table, FileInfo *file, FileInfo *file_old)
{
	PiBool is_replace;

	/* 删除内容为空的文件，但PZB却写入内容为空的一个文件 */
	if(0 == file->file_size)
	{
		pi_dhash_delete(file_table, file, file_old);
		++(pzb->total_file_count);
		return ;
	}

	is_replace = pi_dhash_enter (file_table, file, file_old, NULL);
	/**
	 * 插入还是替换
	 * 插入表示在hash中添加新文件
	 * 替换则是更新hash中的文件
	 */
	if(is_replace)
	{
		/* 防止前面的pzb更新后面的pzb */
		if(file->pzb->major_version < file_old->pzb->major_version)
		{
			pi_dhash_enter (file_table, file_old, file, NULL);
			return ;
		}

		/* 这里判断filelist的更新是否在一个pzb中，然后分别对pzb做操作 */
		if(file_old->pzb == file->pzb)
		{
			pzb->usefull_size -= file_old->length; /* old的数据被替换了，所以pzb的有效长度要减去old的长度 */
		}
		else
		{
			/* 不在一个pzb说明old中的数据被遗弃 */
			--file_old->pzb->usefull_file_count;
			file_old->pzb->usefull_size -=file_old->length;
			file_old->pzb->proportion = (float)file_old->pzb->usefull_size / (float)file_old->pzb->total_size;
		}
	}
	/* 新的pzb有效文件加一 */
	if(file_old->pzb != file->pzb)
	{
		++pzb->usefull_file_count;
	}
	pzb->usefull_size += file->length;
	++(pzb->total_file_count);

	pzb->proportion = (float)pzb->usefull_size / (float)pzb->total_size;
}

/* 将索引信息写入对应的file中 */
static PiBool _write_index_to_file(PiBytes* bytes, FileInfo *file, int64 index_count,  PiBool is_end, PiBytes *pz_bytes)
{
	int64 i = 0;
	char *temp_p = NULL;
	for (i = 0; i < index_count; ++i)
	{
		pi_bytes_read_variable_int(bytes, &file[i].length);
		if (is_end)
		{
			pi_bytes_write_variable_int(&pz_bytes[FILE_SIZE], file[i].length);
		}
	}
	for (i = 0; i < index_count; ++i)
	{
		pi_bytes_read_variable_int(bytes, &file[i].file_size);
		if (is_end)
		{
			pi_bytes_write_variable_int(&pz_bytes[FILE_DATA_SIZE], file[i].file_size);
		}
	}
	for (i = 0; i < index_count; ++i)
	{
		file[i].name_length = pi_bytes_read_int16_unsafe(bytes);
		if (is_end)
		{
			pi_bytes_write_int16(&pz_bytes[FILE_NAME_SIZE], (int16)file[i].name_length);
		}
	}
	for (i = 0; i < index_count; ++i)
	{
		file[i].compression = pi_bytes_read_int8_unsafe(bytes);
		if (is_end)
		{
			pi_bytes_write_int8(&pz_bytes[FILE_COMPRESSION], file[i].compression);
		}
	}
	for (i = 0; i < index_count; ++i)
	{
		file[i].crc = pi_bytes_read_int32_unsafe(bytes);
		if (is_end)
		{
			pi_bytes_write_int32(&pz_bytes[FILE_CRC], file[i].crc);
		}
	}
	for (i = 0; i < index_count; ++i)
	{
		file[i].time = pi_bytes_read_int64_unsafe(bytes);
		if (is_end)
		{
			pi_bytes_write_int64(&pz_bytes[FILE_TIME], file[i].time);
		}
	}
	for (i = 0; i < index_count; ++i)
	{
		pi_bytes_read_data(bytes, &temp_p, EXTEND_INFO_NUM);
		pi_memcpy(file[i].extend_info.info, temp_p, EXTEND_INFO_NUM);
		if (is_end)
		{
			pi_bytes_write_data(&pz_bytes[FILE_EXTEND_INFO], temp_p, EXTEND_INFO_NUM);
		}
	}
	for (i = 0; i < index_count; ++i)
	{
		pi_bytes_read_variable_int(bytes, &file[i].offset);
		if (is_end)
		{
			pi_bytes_write_variable_int(&pz_bytes[FILE_OFFSET], file[i].offset);
		}
	}
	for (i = 0; i < index_count; ++i)
	{
		pi_bytes_read_data(bytes, &temp_p, file[i].name_length);
		pi_memcpy(file[i].file_name, temp_p, file[i].name_length);
		if (is_end)
		{
			pi_bytes_write_data(&pz_bytes[FILE_NAME], temp_p, file[i].name_length);
		}
	}
	return TRUE;
}

/* file_list绑定pzb，file_list写入pz的file_table中 */
static void _init_pzb(PiDhash *file_table, PZB *pzb, char *write_data, IndexInfo index_info, PiBytes *pz_bytes)
{
	int64 i = 0;
	FileInfo *filelist = NULL;
	PiBytes index_bytes;

	pi_bytes_init(&index_bytes);
	pi_bytes_load (&index_bytes, (byte *)write_data, (uint)index_info.size, FALSE);

	filelist = (FileInfo *)pi_malloc0(sizeof(FileInfo) * (uint)index_info.count);
	pzb->offset = index_info.offset;

	_write_index_to_file(&index_bytes, filelist, index_info.count, index_info.is_end, pz_bytes);

	for (i = 0; i < index_info.count; ++i)
	{
		FileInfo file_list_old;
		filelist[i].pzb = pzb;

		update_filelist_hash(pzb, file_table, &filelist[i], &file_list_old);
	}
	pi_free(filelist);
}

/* pz包添加pzb块 */
static void _pz_add_pzb(PZ *pz, PZB *pzb, void *file_handle, IndexInfo index_info, int32 index)
{
	int64 check, file_len = pzb->total_size;
	int64 filelist_index_len = file_len - index_info.offset - END_SIZE;
	char *filelist_index_data = NULL, *p_data = NULL, *write_data = NULL;


	filelist_index_data = (char *)pi_malloc0((uint)(filelist_index_len));
	p_data = filelist_index_data;
	/* todo:效率不足时可用内存映射文件发的方式读 */

	pi_file_read(file_handle, index_info.offset, FALSE, filelist_index_data, (uint)(filelist_index_len));
	check = *((int64 *)p_data);
	p_data += PZB_MASK_SIZE;
	if(check != PZB_MASK)
	{
		pi_free(filelist_index_data);
		return ;
	}
	else
	{
		/* 只有非只读目录和非end_pzb才会放入pzb_vector中 */
		if (index_info.is_end == FALSE && pz->read_only == FALSE)
		{
			if(index < 0)
			{
				pi_vector_push (&pz->pzb_vector, pzb);
			}
			else
			{
				pi_vector_insert(&pz->pzb_vector, index, pzb);
			}
		}

		if (index_info.is_end == TRUE)
		{
			pz->end_pzb = pzb;
		}

		write_data = (char *)pi_malloc0((uint)index_info.size);
		if (index_info.is_compress)
		{
			index_info.size = pi_uncompress(p_data, (uint32)(file_len - index_info.offset - PZB_MASK_SIZE - END_SIZE), write_data, (uint32)index_info.size, LZF);
		}
		else
		{
			pi_memcpy(write_data, p_data, (uint)index_info.size);
		}
		++(pz->pzb_count);

		_init_pzb(&pz->file_table, pzb, write_data, index_info, pz->bytes);

		pi_free(write_data);
		pi_free(filelist_index_data);
	}
}

/* 获得pzb尾部索引 */
static void _get_tail_index(void *file_handle, IndexInfo *tail_index)
{
	char tail[END_SIZE];
	PiBytes index_bytes;
	pi_bytes_init(&index_bytes);

	pi_memset(tail, 0, END_SIZE);

	/* 暂时使用普通读，未使用内催映射文件方式读 */
	/* 从pzb尾部读取tail，即pzb的索引 */
	pi_file_read(file_handle, END_SIZE, TRUE, tail, END_SIZE);
	pi_bytes_load(&index_bytes, (byte *)tail, END_SIZE, FALSE);

	/* 索引数据的位置是的约定的 */
	tail_index->is_compress = (PiBool)pi_bytes_read_int8_unsafe(&index_bytes);
	tail_index->size = pi_bytes_read_int64_unsafe(&index_bytes);
	tail_index->offset = pi_bytes_read_int64_unsafe(&index_bytes);
	tail_index->count = pi_bytes_read_int64_unsafe(&index_bytes);
	tail_index->is_end = pi_bytes_read_int8_unsafe(&index_bytes);

	return ;
}

/* 重建索引，按写入磁盘的文件项目作为判断条件进行文件个数判断。将文件索引和pzb末尾索引直接写到偏移量为offset的地方 */
static PiBool _rebuild_index(void *file_handle)
{
	int64 pzb_size = 0, file_item_len = 0, temp = 0, offset = 0, file_name_len = 0;
	sint pz_bytes_len;
	uint32 index_size = 0, i = 0, total_file_count = 0, temp_data_size = 0;
	uint64 compress_index_size;
	char *file = NULL, *extend_info = NULL, *file_name = NULL, *file_data = NULL, *temp_data = NULL;
	char *compress_index_data = NULL, *temp_file_data = NULL;
	PiBytes pzb_bytes, bytes_tmp, index_bytes;
	PiBytes rebuild_bytes[FILE_COUNT];
	PiBool write_err = FALSE;
	pi_bytes_init(&pzb_bytes);
	pi_bytes_init(&bytes_tmp);
	pi_bytes_init(&index_bytes);

	for (i = 0; i < FILE_COUNT; i++)
	{
		pi_bytes_init(&rebuild_bytes[i]);
	}
	
	pi_file_size(file_handle, &pzb_size);
	if(pzb_size <= 0)
	{
		return FALSE;
	}

	file = (char *)pi_malloc0((uint)pzb_size);
	pi_file_read(file_handle, 0, FALSE, file, (uint)pzb_size);
	pi_bytes_load(&pzb_bytes, (byte *)file, (uint)pzb_size, FALSE);

	pi_bytes_rindex(&pzb_bytes, 0);
	file_item_len = pi_bytes_read_int64_unsafe(&pzb_bytes);
	if(file_item_len > pzb_size || file_item_len < 0)
	{
		return FALSE;
	}

	temp = pzb_size - file_item_len;
	while (temp > 0)
	{
		pi_bytes_write_variable_int(&rebuild_bytes[FILE_SIZE], file_item_len);
		pi_bytes_write_variable_int(&rebuild_bytes[FILE_DATA_SIZE], pi_bytes_read_int64_unsafe(&pzb_bytes));
		file_name_len = pi_bytes_read_int16_unsafe(&pzb_bytes);
		pi_bytes_write_int16(&rebuild_bytes[FILE_NAME_SIZE], (int16)file_name_len);
		pi_bytes_write_int8(&rebuild_bytes[FILE_COMPRESSION], pi_bytes_read_int8_unsafe(&pzb_bytes));
		pi_bytes_write_int32(&rebuild_bytes[FILE_CRC], pi_bytes_read_int32_unsafe(&pzb_bytes));
		pi_bytes_write_int64(&rebuild_bytes[FILE_TIME], pi_bytes_read_int64_unsafe(&pzb_bytes));
		pi_bytes_read_data(&pzb_bytes, &extend_info, EXTEND_INFO_NUM);
		pi_bytes_write_data(&rebuild_bytes[FILE_EXTEND_INFO], extend_info, EXTEND_INFO_NUM);
		pi_bytes_write_variable_int(&rebuild_bytes[FILE_OFFSET], offset);
		pi_bytes_read_data(&pzb_bytes, &file_name, (int)file_name_len);
		pi_bytes_write_data(&rebuild_bytes[FILE_NAME], file_name, (uint)file_name_len);
		pi_bytes_read_data(&pzb_bytes, &file_data, (int)(file_item_len - INDEX_NONAME_NOOFFSET - file_name_len));
		offset += file_item_len;
		total_file_count++;

		file_item_len = pi_bytes_read_int64_unsafe(&pzb_bytes);
		if(file_item_len > pzb_size || file_item_len < 0)
		{
			break;
		}
		temp -= file_item_len;
	}

	pi_bytes_write_int64(&index_bytes, PZB_MASK);

	for (i = 0; i < FILE_COUNT; ++i)
	{
		pi_bytes_rindex (&rebuild_bytes[i], 0);
		pz_bytes_len = pi_bytes_size(&rebuild_bytes[i]);
		index_size += pz_bytes_len;
		pi_bytes_read_data (&rebuild_bytes[i], &temp_data, pz_bytes_len);
		pi_bytes_rindex (&rebuild_bytes[i], 0);
		pi_bytes_write_data(&bytes_tmp, temp_data, pz_bytes_len);
	}
	pi_bytes_read_data(&bytes_tmp, &temp_data, index_size);
	compress_index_data = (char *)pi_malloc0(index_size);
	compress_index_size = pi_compress(temp_data, index_size, compress_index_data, index_size, LZF);

	if (compress_index_size == 0)
	{
		pi_bytes_write_data(&index_bytes, temp_data, index_size);
		pi_bytes_write_int8(&index_bytes, 0);
	}
	else
	{
		pi_bytes_write_data(&index_bytes, compress_index_data, (uint)compress_index_size);
		pi_bytes_write_int8(&index_bytes, 1);
	}
	pi_bytes_write_int64(&index_bytes, index_size);
	pi_free(compress_index_data);

	pi_bytes_write_int64(&index_bytes, offset);
	pi_bytes_write_int64(&index_bytes, total_file_count);
	pi_bytes_write_int8(&index_bytes, 0); /* 重构的pzb不作为end_pzb */

	pi_bytes_rindex (&index_bytes, 0);
	temp_data_size = pi_bytes_size(&index_bytes);	
	pi_bytes_read_data(&index_bytes, &temp_file_data, (int)temp_data_size);

	write_err = pi_file_write(file_handle, offset, FALSE, temp_file_data, (uint)temp_data_size);
	if (!write_err)
	{
		pi_bytes_clear(&pzb_bytes, TRUE);
		pi_bytes_clear(&bytes_tmp, TRUE);
		pi_bytes_clear(&index_bytes, TRUE);
		return FALSE;
	}

	/* 只保留文件正常扫描部分 */
	pi_file_truncate(file_handle, offset + temp_data_size);

	pi_free(file);
	pi_bytes_clear(&pzb_bytes, TRUE);
	pi_bytes_clear(&bytes_tmp, TRUE);
	pi_bytes_clear(&index_bytes, TRUE);
	for (i = 0; i < FILE_COUNT; i++)
	{
		pi_bytes_clear(&rebuild_bytes[i], TRUE);
	}

	return TRUE;
}

void init_contain_pzb(PZ *pz)
{
	PZB *pzb = NULL;
	IndexInfo index_info;
	void *file_handle = NULL;
	wchar *file_path = NULL, *pzb_path = NULL;
	uint i, len = 0;
	PiFileNameInfo file_info;
	void *dir_handle = pi_file_dir_open(pz->path);
	PiVector pzb_vector_tmp;

	pi_vector_init(&pzb_vector_tmp);
	pi_vector_init(&pz->pzb_vector);

	while(pi_file_dir_read(dir_handle, &file_info))
	{
		if(file_info.info.type == FILE_TYPE_REGULAR)
		{
			pzb_path = connect_path(pz->path, file_info.name);
			/* 删除游戏异常时产生的临时文件，否则就算调用了索引重构也会失败 */
			if (pi_wstr_text_index(file_info.name, L".---") > 0 || pi_wstr_text_index(file_info.name, L".!!!") > 0)
			{
				pi_file_delete(pzb_path);
				continue;
			}
			pzb = (PZB *)pi_malloc0(sizeof(PZB));
			pzb->total_size = file_info.info.size;

			pi_wstrcpy(pzb->pzb_path, pzb_path, pi_wstrlen(pzb_path));
			pi_free(pzb_path);

			get_pzb_version(file_info.name, &pzb->major_version, &pzb->minor_version);
			pi_vector_push(&pzb_vector_tmp, pzb);
		}
	}
	pi_file_dir_close(dir_handle);

	pi_vector_sort(&pzb_vector_tmp, _sort_pzb_by_version, NULL);

	len = pi_vector_size(&pzb_vector_tmp);
	for(i = 0; i < len; ++i)
	{
		PZB *pzb_temp = (PZB *)pi_vector_get(&pzb_vector_tmp, i);
		file_handle = pi_file_open(pzb_temp->pzb_path, FILE_OPEN_WRITE | FILE_OPEN_READ);
		pi_free(file_path);

		if(NULL == file_handle)
		{
			continue;
		}

		_get_tail_index(file_handle, &index_info);

		/* 如果非终结块发现块的偏移大于文件长度，表示块出错, 对该块重建索引写入 */
		if (index_info.offset > pzb_temp->total_size || index_info.offset < 0)
		{
			if (!_rebuild_index(file_handle))
			{
				/* 遇见重构都无法恢复的PZB，就不在读取该PZB */
				pi_error_set(ERROR_TYPE_DATA_ILLEGAL, 0, NULL, __FILE__, __LINE__);
				continue; 
			}
			
			_get_tail_index(file_handle, &index_info);

			pi_file_size(file_handle, &pzb_temp->total_size);
		}

		_pz_add_pzb(pz, pzb_temp, file_handle, index_info, -1);

		pi_file_close(file_handle);
	}
	pi_vector_clear(&pzb_vector_tmp, TRUE);
}