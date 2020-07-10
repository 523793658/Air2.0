#include "pz_merge.h"
#include <pi_compress.h>
#include <stdio.h>


/* 获得合并的pzb句柄 */
static void*  PI_API _get_merge_pzb_handle(PZ *impl, PZB *pzb)
{
	wchar *merge_pzb_name = NULL;
	void *write_handle;
	int64 file_size = 0;

	pzb->minor_version += 1;
	merge_pzb_name = set_pzb_name(pzb->major_version, pzb->minor_version);
	merge_pzb_name = connect_path(impl->path, merge_pzb_name);
	write_handle = pi_file_open(merge_pzb_name, FILE_OPEN_WRITE | FILE_OPEN_READ);
	pi_free(merge_pzb_name);

	/* 防止多线程重复合并出同一文件 */
	pi_file_size(write_handle, &file_size);
	if (file_size > 0)
	{
		return NULL;
	}

	return write_handle;
}

int64 PI_API fill_merge_pzb_index(PiBytes *bytes, int64 write_offset)
{
	int64 bytes_len, compress_len, file_offset = 0, index_count = 0;
	char *temp_data = NULL, *file_data;
	int32 i = 0;
	FileInfo temp_file;
	PiBytes index_bytes, temp_bytes[FILE_COUNT];
	char *compress_index = NULL;
	PiBool is_compress = TRUE;
	pi_bytes_init(&index_bytes);

	pi_memset(&temp_file, 0, sizeof(FileInfo));
	for (i = 0; i < FILE_COUNT; ++i)
	{
		pi_bytes_init(&temp_bytes[i]);
	}
	pi_bytes_write_int64(bytes, PZB_MASK);

	/* 从缓冲中读取文件索引数据，接着会尝试压缩。同时得到文件个数 */
	while (file_offset < write_offset)
	{
		init_fileinfo_from_bytes(bytes, &temp_file, FALSE);
		pi_bytes_write_variable_int(&temp_bytes[FILE_SIZE], temp_file.length);
		pi_bytes_write_variable_int(&temp_bytes[FILE_DATA_SIZE], temp_file.file_size);
		pi_bytes_write_int16(&temp_bytes[FILE_NAME_SIZE], (int16)temp_file.name_length);
		pi_bytes_write_int8(&temp_bytes[FILE_COMPRESSION], temp_file.compression);
		pi_bytes_write_int32(&temp_bytes[FILE_CRC], temp_file.crc);
		pi_bytes_write_int64(&temp_bytes[FILE_TIME], temp_file.time);
		pi_bytes_write_data(&temp_bytes[FILE_EXTEND_INFO], temp_file.extend_info.info, EXTEND_INFO_NUM);
		pi_bytes_write_variable_int(&temp_bytes[FILE_OFFSET], file_offset);
		pi_bytes_write_data(&temp_bytes[FILE_NAME], temp_file.file_name, temp_file.name_length);
		/* 将文件的data从bytes读出，虽然没用到，但是需要通过这样定位到下一个文件的索引处 */
		pi_bytes_read_data(bytes, &file_data, (int)(temp_file.length - INDEX_NONAME_NOOFFSET - temp_file.name_length));
		file_offset += temp_file.length;
		++index_count;
	}

	for (i = 0; i < FILE_COUNT; ++i)
	{
		pi_bytes_copy(&index_bytes, &temp_bytes[i]);
		pi_bytes_clear(&temp_bytes[i], TRUE);
	}

	pi_bytes_rindex(bytes, 0);
	bytes_len = pi_bytes_size(&index_bytes);
	pi_bytes_read_data(&index_bytes, &temp_data, (int)bytes_len);
	compress_index = (char *)pi_malloc0((uint)bytes_len);
	compress_len = pi_compress(temp_data, (uint32)bytes_len, compress_index, (uint32)bytes_len, LZF);

	if (compress_len == 0)
	{
		is_compress = FALSE;
		compress_len = bytes_len;
	}

	if (is_compress)
	{
		pi_bytes_write_data(bytes, compress_index, (uint)compress_len);
	}
	else
	{
		pi_bytes_write_data(bytes, temp_data, (uint)bytes_len);
	}

	pi_bytes_clear(&index_bytes, TRUE);
	pi_free(compress_index);

	/* 将尾部索引写入缓冲，该索引指向文件索引 */
	pi_bytes_write_int8(bytes, (int8)is_compress);
	pi_bytes_write_int64(bytes, bytes_len);
	pi_bytes_write_int64(bytes, file_offset);
	pi_bytes_write_int64(bytes,index_count);
	pi_bytes_write_int8(bytes, 0);

	return 0;
}

PiSelectR PI_API foreach_valid_file(void *data, void *value)
{
	MergeData *merge_data = (MergeData *)data;
	FileInfo *file = (FileInfo *)value;
	FileInfo *file_temp = NULL;

	/* 查找file对应pzb的合并标志位 */
	if (file->pzb->is_merge)
	{
		file_temp = (FileInfo *)pi_malloc0(sizeof(FileInfo));
		pi_memcpy(file_temp, file, sizeof(FileInfo));
		pi_vector_push(&merge_data->filelist, file_temp);
	}	
	return SELECT_NEXT;
}

/* 获得这次合并的fileinfo */
static void PI_API _get_merge_filelist(MergeData *merge_data, PiVector *filelist)
{
	int file_len, pzb_len;
	int i, j;
	FileInfo *file_loop, *file_temp = NULL;
	PZB *pzb_loop;
	file_len = pi_vector_size(&merge_data->filelist);
	pzb_len = pi_vector_size(&merge_data->pzblist);
	for(i = 0; i < file_len; i++)
	{
		file_loop = (FileInfo *)pi_vector_get(&merge_data->filelist, i);
		for (j = 0; j < pzb_len; j++)
		{
			pzb_loop = (PZB *)pi_vector_get(&merge_data->pzblist, j);
			if(pi_wstr_equal(file_loop->pzb->pzb_path, pzb_loop->pzb_path, FALSE))
			{
				file_temp = (FileInfo *)pi_malloc0(sizeof(FileInfo));
				pi_memcpy(file_temp, file_loop, sizeof(FileInfo));
				pi_vector_push(filelist, file_temp);
			}
		}
	}
	pi_vector_clear(&merge_data->pzblist, FALSE);
	pi_vector_init(&merge_data->pzblist);
	return ;
}

/* 合并pzb的实现 */
void PI_API _merge_to_pzb(MergeData *merge_data, PZ *impl)
{
	PZB *last_pzb = (PZB *)pi_vector_peek(&merge_data->pzblist);
	FileInfo *file = NULL, *file_next = NULL;
	void *merge_pzb_handle = NULL, *read_handle = NULL;
	int64 read_offset, read_length, file_size = 0, write_offset = 0;
	sint read_err = 0;
	char *data = NULL;
	PiVector merge_filelist;
	PiBytes bytes;

	pi_bytes_init(&bytes);
	pi_vector_init(&merge_filelist);

	/** 
	 * 合并的pzb小于一个就不合并,并将is_merge置为0防止被删除
	 * 重置pzb的vector，不然将会合并到下一次
	 */
	if(pi_vector_size(&merge_data->pzblist) <= 1)
	{
		PZB *tmp = (PZB *)pi_vector_peek(&merge_data->pzblist);
		tmp->is_merge = 0;
		pi_vector_clear(&merge_data->pzblist, FALSE);
		pi_vector_init(&merge_data->pzblist);
		return ;
	}

	_get_merge_filelist(merge_data, &merge_filelist);
	if(!pi_vector_size(&merge_filelist) > 0)
		return ;

	file = (FileInfo *)pi_vector_pop(&merge_filelist);
	file_next = (FileInfo *)pi_vector_pop(&merge_filelist);
	if (file)
	{
		read_offset = file->offset;
		read_length = file->length;

		while(file != NULL)
		{
			read_handle = pi_file_open(file->pzb->pzb_path, FILE_OPEN_READ);

			/* 如果若干相邻文件在同一个pzb的相邻位置，则一次读取写入他们 */
			if(file_next != NULL && file->pzb == file_next->pzb && file->offset + file->length == file_next->offset)
			{
				read_length += file_next->length;
				pi_free(file);
				file = file_next;
				file_next = (FileInfo *)pi_vector_pop(&merge_filelist);
				pi_file_close(read_handle);
				continue;
			}

			data = (char *)pi_malloc0((uint)(read_length + 1));
			read_err = pi_file_read(read_handle, read_offset, FALSE, data, (uint)read_length);
			pi_file_close(read_handle);
			if (read_err <= 0)
			{
				pi_free(data);
				continue;
			}

			pi_bytes_write_data(&bytes, data, (uint)read_length);
			pi_free(data);

			write_offset += read_length;

			pi_free(file);
			file = file_next;
			file_next = (FileInfo *)pi_vector_pop(&merge_filelist);

			if(file != NULL)
			{
				read_offset = file->offset;
				read_length = file->length;
			}
		}

		fill_merge_pzb_index(&bytes, write_offset);

		pi_bytes_rindex(&bytes, 0);
		file_size = pi_bytes_size(&bytes);
		pi_bytes_read_data(&bytes, &data, (uint)file_size);

		merge_pzb_handle = _get_merge_pzb_handle(impl, last_pzb);
		if(merge_pzb_handle == NULL)
		{
			return ;
		}

		pi_file_write(merge_pzb_handle, 0, FALSE, data, (uint)file_size);
		pi_bytes_clear(&bytes, TRUE);
		pi_file_close(merge_pzb_handle);
	}
}

void PI_API pz_merge(void *pz, int64 time)
{
	PZ *impl = (PZ *)pz;
	PZB *pzb = NULL, *pzb_tmp = NULL;
	PiVector *pzb_vector = &impl->pzb_vector;
	int vector_length = 0;
	int64 i = 0, dec_num = 0, merge_files_num = 0, merge_useful_size = 0;
	PiBytes bytes;
	MergeData *merge_data = (MergeData *)pi_malloc0(sizeof(MergeData));

	vector_length = pi_vector_size(pzb_vector);
	
	if (vector_length <= 1)
	{
		return;
	}

	for (i = 0; i < vector_length; ++i)
	{
		pzb = (PZB *)pi_vector_get(pzb_vector, (uint)i);

		if (pzb->proportion < PZB_MERGE_SCALE)
		{
			pzb->is_merge = 1;
			++dec_num;
		}
	}
	if (dec_num < 1)
	{
		return;
	}

	pi_lock_read_lock(&impl->wlock, &impl->rlock, LOCK_COUNT);

	pi_dhash_foreach(&impl->file_table, foreach_valid_file, merge_data);

	pi_lock_read_free(&impl->rlock);

	for (i = 0; i < vector_length; ++i)
	{
		pzb = (PZB *)pi_vector_get(pzb_vector, (uint)i);
		if (1 == pzb->is_merge)
		{
			pzb_tmp = (PZB *)pi_malloc0(sizeof(PZB));
			pi_memcpy(pzb_tmp, pzb, sizeof(PZB));
			pi_vector_push(&merge_data->pzblist, pzb_tmp);

			merge_files_num += pzb->usefull_file_count;
			merge_useful_size += pzb->usefull_size;
			if(merge_files_num >= PZB_FILE_COUNT || merge_useful_size >= PZB_SIZE)
			{
				pi_vector_pop(&merge_data->pzblist);
				_merge_to_pzb(merge_data, impl);

				merge_files_num = 0;
				merge_useful_size = 0;
				pi_vector_push(&merge_data->pzblist, pzb_tmp);
				merge_files_num += pzb_tmp->usefull_file_count;
				merge_useful_size += pzb_tmp->usefull_size;
			}
		}
	}

	pi_bytes_init(&bytes);
	/* 根据pzb->index来进行删除 */
	for (i = 0; i < vector_length; ++i)
	{	
		pzb = (PZB *)pi_vector_get(pzb_vector, (uint)i);
		/* pzb->proportion也要为0时才删除，防止出现单个pzb未合并但被删除情况 */
		if(pzb->is_merge && pzb->proportion > -FLOAT_ZERO && pzb->proportion < FLOAT_ZERO)
		{
			pi_bytes_write_wstr(&bytes, pzb->pzb_path);
		}
	}

	/* 将新合并的pzb内容更新到pz的file_table,然后删除合并前的pzb
	   不然新合并的pzb要下次初始化后才能使用 */
	init_contain_pzb(impl);
	delete_merged_pzb(impl, &bytes);
}