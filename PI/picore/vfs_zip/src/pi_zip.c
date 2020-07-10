
#include "pi_zip.h"
#include <windows.h>
#include "zip_format.h"
#include "pi_compress.h"

#define MAX_COMMENT_LENGTH (0xffff)										/* 文件的注释的最大长度 */

#define BUF_READ_COMMENT (0x400)

struct PiZipFile
{
	wchar *name;
	int64 time;
	uint32 crc;
	uint32 uncompressed_size;

	/* 解压相关的参数 */
	ZipCompressionMethod compression_method;
	uint32 compressed_data_offset;
	uint32 compressed_size;
};

struct PiZipDir
{
	wchar *name;
	int64 time;
	PiVector sub_dirs;
	PiVector files;
};

struct PiZip
{
	PiVector sub_dirs;
	PiVector files;

	PiDvector all_sub_dirs;
	PiDvector all_files;
	void *handle;
};

/* 从字节流中读出文件名 */
static wchar *_str_to_wstr(const char *src, uint len)
{
	char *src_temp = pi_malloc0(len + 1);
	pi_strcpy(src_temp, src, len);
	/* TODO: 编码转换 */
	return pi_str_to_wstr(src_temp, PI_CP_GBK);
}

/* 通过zip的文件句柄找到zip的中心目录结束符eocd_record */
static PiBool _search_eocd_record(const void *handle, EndOfCentralDirectoryRecord *eocd_record)
{
	char *buf;
	int64 file_size;
	int64 back_read = EOCD_RECORD_SIZE;
	int64 max_back = MAX_COMMENT_LENGTH + EOCD_RECORD_SIZE;
	if (!pi_file_size(handle, &file_size))
	{
		return FALSE;
	}

	if (max_back > file_size)
	{
		max_back = file_size;
	}

	buf = pi_malloc(EOCD_RECORD_SIZE + BUF_READ_COMMENT);

	while (back_read < max_back)
	{
		sint len, i;
		int64 read_pos;
		uint read_size = EOCD_RECORD_SIZE;
		if (back_read + BUF_READ_COMMENT > max_back)
		{
			read_size += (uint)(max_back - back_read);
			back_read = max_back;
		}
		else
		{
			read_size += BUF_READ_COMMENT;
			back_read += BUF_READ_COMMENT;
		}

		read_pos = file_size - back_read;

		len = pi_file_read(handle, read_pos, FALSE, buf, read_size);

		if (-1 == len)
		{
			break;
		}

		for (i = len - EOCD_RECORD_SIZE; i >= 0; i--)
		{
			if (buf[i] == 0x50 &&
				buf[i + 1] == 0x4b &&
				buf[i + 2] == 0x05 &&
				buf[i + 3] == 0x06)
			{
				pi_memcpy(eocd_record, buf + i + 4, sizeof(EndOfCentralDirectoryRecord));

				if (read_pos + i + EOCD_RECORD_SIZE + eocd_record->comment_length == file_size)
				{
					pi_free(buf);
					return TRUE;
				}
			}
		}
	}

	/* 没有找到中心目录结束标记符 */
	pi_free(buf);

	return FALSE;
}

static int64 _dos_time_to_file_time(uint16 dos_time_date, uint16 dos_time_time)
{
	uint64 time;
	FILETIME file_time;
	DosDateTimeToFileTime(dos_time_date, dos_time_time, &file_time);
	time = file_time.dwHighDateTime;
	time = file_time.dwLowDateTime | (time << 32);
	return time;
}

static void _add_entry(PiZip *zip, CentralDirectoryHeader *dir_header, char *file_name)
{
	if (dir_header->external_attributes & ZIP_A_ARCH)
	{
		PiZipFile file;

		file.time = _dos_time_to_file_time(dir_header->date, dir_header->time);
		file.crc = dir_header->crc32;
		file.uncompressed_size = dir_header->uncompressed_size;

		file.compressed_data_offset = dir_header->header_offset + sizeof(LocalFileHeader);
		file.compressed_size = dir_header->compressed_size;
		/* TODO: 从flag中获取压缩参数 */
		file.compression_method = dir_header->compression_method;

		file.name = _str_to_wstr(file_name, dir_header->file_name_length);

		pi_dvector_push(&zip->all_files, &file);
	}
	else if (dir_header->external_attributes & ZIP_A_SUBDIR)
	{
		PiZipDir dir;

		pi_vector_init(&dir.sub_dirs);
		pi_vector_init(&dir.files);
		dir.time = _dos_time_to_file_time(dir_header->date, dir_header->time);

		/* 文件夹以一个正斜杠"/"结尾，需要去掉 */
		dir.name = _str_to_wstr(file_name, dir_header->file_name_length - 1);

		pi_dvector_push(&zip->all_sub_dirs, &dir);
	}
}

static PiCompR PI_API _file_name_comp(void *user_data, const PiZipFile *a, const PiZipFile *b)
{
	return pi_wstr_compare(a->name, b->name);
}

static PiCompR PI_API _dir_name_comp(void *user_data, const PiZipDir *a, const PiZipDir *b)
{
	return pi_wstr_compare(a->name, b->name);
}

static PiBool _is_father_path(wchar *father, wchar *child, sint index)
{
	sint i;
	for (i = index - 1; i >= 0; i--)
	{
		if (father[i] != child[i])
		{
			return FALSE;
		}
	}
	return TRUE;
}

static void _generate_tree(PiZip *zip)
{
	uint i, size;

	pi_dvector_sort(&zip->all_sub_dirs, _dir_name_comp, NULL);
	size = pi_dvector_size(&zip->all_sub_dirs);
	for (i = 0; i < size; i++)
	{
		PiZipDir *current_dir = pi_dvector_get(&zip->all_sub_dirs, i);

		sint index = pi_wstr_char_last_index(current_dir->name, pi_wstrlen(current_dir->name), DIR_SEPARATOR);

		if (index == -1)
		{
			pi_vector_push(&zip->sub_dirs, current_dir);
		}
		else
		{
			uint j;
			for (j = 0; j < size; j++)
			{
				PiZipDir *dir = pi_dvector_get(&zip->all_sub_dirs, j);

				if (_is_father_path(dir->name, current_dir->name, index))
				{
					pi_vector_push(&dir->sub_dirs, current_dir);
					break;
				}
			}
		}
	}

	pi_dvector_sort(&zip->all_files, _file_name_comp, NULL);
	size = pi_dvector_size(&zip->all_files);
	for (i = 0; i < size; i++)
	{
		PiZipFile *current_file = pi_dvector_get(&zip->all_files, i);

		sint index = pi_wstr_char_last_index(current_file->name, pi_wstrlen(current_file->name), DIR_SEPARATOR);

		if (index == -1)
		{
			pi_vector_push(&zip->files, current_file);
		}
		else
		{
			uint j;
			for (j = 0; j < size; j++)
			{
				PiZipDir *dir = pi_dvector_get(&zip->all_sub_dirs, j);

				if (_is_father_path(dir->name, current_file->name, index))
				{
					pi_vector_push(&dir->files, current_file);
					break;
				}
			}
		}
	}
}

static uint16 _get_uint16(byte *buffer)
{
	uint16 a = buffer[1];
	a = buffer[0] | (a << 8);
	return a;
}

static uint32 _get_uint32(byte *buffer)
{
	uint32 a = buffer[3];
	a = buffer[2] | (a << 8);
	a = buffer[1] | (a << 8);
	a = buffer[0] | (a << 8);
	return a;
}

static void _parse_central_directory_header(CentralDirectoryHeader *dir_header, byte *buffer)
{
	dir_header->version = _get_uint16(buffer);
	dir_header->version_needed = _get_uint16(buffer + 2);
	dir_header->flag = _get_uint16(buffer + 4);
	dir_header->compression_method = _get_uint16(buffer + 6);
	dir_header->time = _get_uint16(buffer + 8);
	dir_header->date = _get_uint16(buffer + 10);
	dir_header->crc32 = _get_uint32(buffer + 12);
	dir_header->compressed_size = _get_uint32(buffer + 16);
	dir_header->uncompressed_size = _get_uint32(buffer + 20);
	dir_header->file_name_length = _get_uint16(buffer + 24);
	dir_header->extra_field_length = _get_uint16(buffer + 26);
	dir_header->file_comment_length = _get_uint16(buffer + 28);
	dir_header->disk_number_start = _get_uint16(buffer + 30);
	dir_header->internal_attributes = _get_uint16(buffer + 32);
	dir_header->external_attributes = _get_uint32(buffer + 34);
	dir_header->header_offset = _get_uint32(buffer + 38);
}

static void _parse_local_file_header(LocalFileHeader *file_header, byte *buffer)
{
	file_header->version_needed = _get_uint16(buffer);
	file_header->flag = _get_uint16(buffer + 2);
	file_header->compression_method = _get_uint16(buffer + 4);
	file_header->time = _get_uint16(buffer + 6);
	file_header->date = _get_uint16(buffer + 8);
	file_header->crc32 = _get_uint32(buffer + 10);
	file_header->compressed_size = _get_uint32(buffer + 14);
	file_header->uncompressed_size = _get_uint32(buffer + 18);
	file_header->file_name_length = _get_uint16(buffer + 22);
	file_header->extra_field_length = _get_uint16(buffer + 24);
}

static PiBool _search_central_directory(PiZip *zip, EndOfCentralDirectoryRecord *eocd_record)
{
	uint32 i;
	LocalFileHeader file_header;
	CentralDirectoryHeader dir_header;
	uint32 data_descriptor_buffer[4];
	byte file_buffer[LOCAL_FILE_HEADER_WITH_SIGNATURE_SIZE];

	char *buffer = pi_malloc(eocd_record->directory_size);
	byte *dir_buffer = (byte *)buffer;

	/* 根据中心目录结束符中记录的中心目录偏移量和长度来读取整个中心目录 */
	if (-1 == pi_file_read(zip->handle, eocd_record->directory_offset, FALSE, buffer, eocd_record->directory_size))
	{
		pi_free(buffer);
		return FALSE;
	}

	for (i = 0; i < eocd_record->entries_on_disk; i++)
	{
		if (dir_buffer[0] != 0x50 ||
			dir_buffer[1] != 0x4b ||
			dir_buffer[2] != 0x01 ||
			dir_buffer[3] != 0x02)
		{
			pi_free(buffer);
			return FALSE;
		}

		_parse_central_directory_header(&dir_header, dir_buffer + 4);
		dir_buffer += CENTRAL_DIRECTORY_HEADER_WITH_SIGNATURE_SIZE;

		pi_file_read(zip->handle, dir_header.header_offset, FALSE, (char *)file_buffer, LOCAL_FILE_HEADER_WITH_SIGNATURE_SIZE);

		if (file_buffer[0] != 0x50 ||
			file_buffer[1] != 0x4b ||
			file_buffer[2] != 0x03 ||
			file_buffer[3] != 0x04)
		{
			pi_free(buffer);
			return FALSE;
		}

		_parse_local_file_header(&file_header, file_buffer + 4);

		if (dir_header.version_needed != file_header.version_needed ||
			dir_header.flag != file_header.flag ||
			dir_header.compression_method != file_header.compression_method ||
			dir_header.time != file_header.time ||
			dir_header.date != file_header.date ||
			dir_header.file_name_length != file_header.file_name_length)
		{
			pi_free(buffer);
			return FALSE;
		}

		if (file_header.flag & ZIP_SUMS_FOLLOW)
		{
			uint32 data_descriptor_offset;

			if (file_header.crc32 != 0 ||
				file_header.compressed_size != 0 ||
				file_header.uncompressed_size != 0)
			{
				pi_free(buffer);
				return FALSE;
			}

			data_descriptor_offset = dir_header.header_offset + LOCAL_FILE_HEADER_WITH_SIGNATURE_SIZE + dir_header.compressed_size;
			pi_file_read(zip->handle, dir_header.header_offset, FALSE, (char *)data_descriptor_buffer, sizeof(uint32) * 4);

			if (data_descriptor_buffer[0] == DATA_DESCRIPTOR_SIGNATURE)
			{
				file_header.crc32 = data_descriptor_buffer[1];
				file_header.compressed_size = data_descriptor_buffer[2];
				file_header.uncompressed_size = data_descriptor_buffer[3];
			}
			else
			{
				file_header.crc32 = data_descriptor_buffer[0];
				file_header.compressed_size = data_descriptor_buffer[1];
				file_header.uncompressed_size = data_descriptor_buffer[2];
			}
		}

		if (dir_header.crc32 != file_header.crc32 ||
			dir_header.compressed_size != file_header.compressed_size ||
			dir_header.uncompressed_size != file_header.uncompressed_size)
		{
			pi_free(buffer);
			return FALSE;
		}

		_add_entry(zip, &dir_header, (char *)dir_buffer);
		dir_buffer += dir_header.file_name_length + dir_header.extra_field_length + dir_header.file_comment_length;
	}

	_generate_tree(zip);

	return TRUE;
}

PiZip *PI_API pi_zip_open(const wchar *name, PiFileOpenMode mode)
{
	PiZip *zip;
	EndOfCentralDirectoryRecord eocd_record;

	void *handle = pi_file_open(name, mode);				/* 打开文件 */
	if (!handle)
	{
		return NULL;
	}

	if (!_search_eocd_record(handle, &eocd_record))			/* 查找中心目录结束符 */
	{
		pi_error_set(ERROR_TYPE_INTERNAL, 0, L"zip file end eocd_record not found, file maybe is no a zip or corrupted", __FILE__, __LINE__);
		return NULL;
	}

	zip = pi_new(PiZip, 1);
	pi_dvector_init(&zip->all_files, sizeof(PiZipFile));
	pi_dvector_init(&zip->all_sub_dirs, sizeof(PiZipDir));
	pi_vector_init(&zip->files);
	pi_vector_init(&zip->sub_dirs);
	zip->handle = handle;

	if (!_search_central_directory(zip, &eocd_record))
	{
		pi_error_set(ERROR_TYPE_INTERNAL, 0, L"zip file end central directory not found, file maybe is no a zip or corrupted", __FILE__, __LINE__);
		pi_free(zip);
		return NULL;
	}

	return zip;
}

PiBool PI_API pi_zip_close(PiZip *zip)
{
	uint i, size;
	if (!pi_file_close(zip->handle))
	{
		pi_free(zip);
		return FALSE;
	}

	size = pi_dvector_size(&zip->all_sub_dirs);
	for (i = 0; i < size; i++)
	{
		PiZipDir *dir = pi_dvector_get(&zip->all_sub_dirs, i);
		pi_vector_clear(&dir->files, TRUE);
		pi_vector_clear(&dir->sub_dirs, TRUE);
		pi_free(dir->name);
	}
	pi_dvector_clear(&zip->all_sub_dirs, TRUE);

	size = pi_dvector_size(&zip->all_files);
	for (i = 0; i < size; i++)
	{
		PiZipFile *file = pi_dvector_get(&zip->all_files, i);
		pi_free(file->name);
	}
	pi_dvector_clear(&zip->all_files, TRUE);

	pi_free(zip);
	return TRUE;
}

static PiCompR PI_API _dir_name_compare(const wchar *name, const void *a, const PiZipDir *b)
{
	return pi_wstr_compare(name, b->name);
}

static PiZipDir *_search_dir(PiZip *zip, const wchar *name)
{
	uint index;
	if (0 == pi_binary_search(pi_dvector_address(&zip->all_sub_dirs), sizeof(PiZipDir), pi_dvector_size(&zip->all_sub_dirs), NULL, _dir_name_compare, (void *)name, &index))
	{
		return pi_dvector_get(&zip->all_sub_dirs, index);
	}
	return NULL;
}

static PiCompR PI_API _file_name_compare(const wchar *name, const void *a, const PiZipFile *b)
{
	return pi_wstr_compare(name, b->name);
}

static PiZipFile *_search_file(PiZip *zip, const wchar *name)
{
	uint index;
	if (0 == pi_binary_search(pi_dvector_address(&zip->all_files), sizeof(PiZipFile), pi_dvector_size(&zip->all_files), NULL, _file_name_compare, (void *)name, &index))
	{
		return pi_dvector_get(&zip->all_files, index);
	}
	return NULL;
}

PiBool PI_API pi_zip_item_get_info(PiZip *zip, const wchar *name, PiFileInfo *info)
{
	PiZipDir *dir;
	PiZipFile *file = _search_file(zip, name);
	if (file)
	{
		info->crc = file->crc;
		info->size = file->uncompressed_size;
		info->time_create = info->time_write = file->time;
		info->type = FILE_TYPE_REGULAR;
		return TRUE;
	}
	dir = _search_dir(zip, name);
	if (dir)
	{
		info->time_create = info->time_write = file->time;
		info->type = FILE_TYPE_DIRECTORY;
		return TRUE;
	}

	return FALSE;
}

int64 PI_API pi_zip_item_get_size(PiZip *zip, const wchar *name)
{
	PiZipDir *dir;
	PiZipFile *file = _search_file(zip, name);
	if (file)
	{
		return file->uncompressed_size;
	}
	dir = _search_dir(zip, name);
	if (dir)
	{
		return 0;
	}

	return -1;
}

PiBool PI_API pi_zip_dir_is_empty(PiZip *zip, const wchar *name, PiBool *is_empty)
{
	PiZipDir *dir = _search_dir(zip, name);
	if (dir)
	{
		*is_empty = pi_vector_size(&dir->sub_dirs) > 0;
		return TRUE;
	}

	return FALSE;
}

PiZipDir *PI_API pi_zip_dir_open(PiZip *zip, const wchar *name)
{
	return _search_dir(zip, name);
}

PiBool PI_API pi_zip_dir_read(PiZipDir *dir, uint32 index, PiFileNameInfo *info)
{
	uint sub_dir_count = pi_vector_size(&dir->sub_dirs);
	if (index < sub_dir_count)
	{
		PiZipDir *sub_dir = pi_vector_get(&dir->sub_dirs, index);
		info->info.crc = 0;
		info->info.size = 0;
		info->info.time_create = sub_dir->time;
		info->info.time_write = sub_dir->time;
		info->info.type = FILE_TYPE_DIRECTORY;

		pi_wstrcpy(info->name, sub_dir->name, pi_wstrlen(sub_dir->name));
	}
	else if (index - sub_dir_count < pi_vector_size(&dir->files))
	{
		PiZipFile *file = pi_vector_get(&dir->files, index - sub_dir_count);
		info->info.crc = file->crc;
		info->info.size = file->uncompressed_size;
		info->info.time_create = file->time;
		info->info.time_write = file->time;
		info->info.type = FILE_TYPE_REGULAR;

		pi_wstrcpy(info->name, file->name, pi_wstrlen(file->name));
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

PiBool PI_API pi_zip_get_crc(PiZip *zip, const wchar *name, uint32 *crc)
{
	PiZipFile *file = _search_file(zip, name);
	if (file)
	{
		*crc = file->crc;
		return TRUE;
	}

	return FALSE;
}

PiZipFile *PI_API pi_zip_file_open(PiZip *zip, const wchar *name)
{
	return _search_file(zip, name);
}

uint32 PI_API pi_zip_file_get_size(PiZipFile *file)
{
	return file->uncompressed_size;
}

uint32 PI_API pi_zip_file_get_crc(PiZipFile *file)
{
	return file->crc;
}

sint PI_API pi_zip_file_read(PiZip *zip, const PiZipFile *file, int64 offset, PiBool from_end, char *data, uint len)
{
	sint read_len;
	int64 file_size = file->uncompressed_size;

	if (from_end)
	{
		offset = file_size - offset;
	}

	if (offset < 0 || offset > file_size)
	{
		pi_error_set(ERROR_TYPE_OUT_OF_BOUNDS, 0, NULL, __FILE__, __LINE__);
		return -1;
	}

	read_len = MIN(len, (uint)(file_size - offset));

	/* compression method IS STORED */
	if (ZIP_METHOD_STORE == file->compression_method)
	{
		return pi_file_read(zip->handle, file->compressed_data_offset + offset, FALSE, data, read_len);
	}
	/* compression method IS DEFLATE */
	else if (ZIP_METHOD_DEFLATE == file->compression_method)
	{
		char *compressed_buf = pi_malloc(file->compressed_size);
		pi_file_read(zip->handle, file->compressed_data_offset, FALSE, compressed_buf, file->compressed_size);

		/* 读取整个文件，直接把解压缩数据流输出到data */
		if (offset == 0 && len >= file->uncompressed_size)
		{
			if (!pi_uncompress(compressed_buf, file->compressed_size, data, file->uncompressed_size, DEFLATE))
			{
				pi_free(compressed_buf);
				return -1;
			}
		}
		else
		{
			char *uncompressed_buf = pi_malloc(file->uncompressed_size);
			if (!pi_uncompress(compressed_buf, file->compressed_size, uncompressed_buf, file->uncompressed_size, DEFLATE))
			{
				pi_free(compressed_buf);
				pi_free(uncompressed_buf);
				return -1;
			}
			pi_memcpy(data, uncompressed_buf + offset, read_len);
			pi_free(uncompressed_buf);
		}

		pi_free(compressed_buf);
	}
	else
	{
		pi_error_set(ERROR_TYPE_INTERNAL, 0, L"Unsupported Compression Method", __FILE__, __LINE__);
		return -1;
	}

	return read_len;
}
