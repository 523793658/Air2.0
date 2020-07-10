
#include "pi_zip.h"

/* VFS中的zip中的目录的句柄 */
typedef struct
{
	wchar name[NAME_LENGTH];
	PiZipDir *handle;				/* zip库中的zip中的文件的句柄 */
	uint index;						/* 当前子项的索引 */
} VfsZipDir;

/* VFS中的zip中的文件的句柄 */
typedef struct
{
	wchar name[NAME_LENGTH];
	PiZipFile *handle;				/* zip库中的zip中的文件的句柄 */
	PiFileOpenMode mode;
} VfsZipFile;

/* VFS中的zip的句柄 */
typedef struct
{
	wchar name[NAME_LENGTH];
	PiFileOpenMode mode;

	PiZip *handle;					/* zip句柄 */
	PiBool valid;					/* 有效性 */
	sint handle_lock;				/* 互斥锁，避免多个线程同时打开ZIP文件句柄 */

	int64 time;

	PiVector files;
	sint files_lock;

	PiVector directories;
	sint directories_lock;

	sint write_lock;
	sint read_lock;
} VfsZip;

/* 打开zip文件，每个方法执行先都必须运行一次，以确保zip打开 */
static void _reload_zip(VfsZip *zip)
{
	pi_lock_spin_lock(&zip->handle_lock, LOCK_COUNT);

	if (!zip->valid)
	{
		zip->handle = pi_zip_open(zip->name, zip->mode);
		if (zip->handle)
		{
			uint32 i, size;

			size = pi_vector_size(&zip->files);
			for (i = 0; i < size; i++)
			{
				VfsZipFile *file = pi_vector_get(&zip->files, i);
				file->handle = pi_zip_file_open(zip->handle, file->name);
			}

			size = pi_vector_size(&zip->directories);
			for (i = 0; i < size; i++)
			{
				VfsZipDir *dir = pi_vector_get(&zip->directories, i);
				dir->handle = pi_zip_dir_open(zip->handle, dir->name);
			}

			zip->valid = TRUE;
		}
		else
		{
			zip->valid = FALSE;
		}
	}

	/* 添加时间戳 */
	zip->time = pi_time_now();

	pi_lock_spin_free(&zip->handle_lock);
}

/* 获得文件信息，将获得模块对应目录的文件信息，返回是否成功，失败可查错误编号 */
static PiBool PI_API _get_info(VfsZip *zip, const wchar *name, PiFileInfo *info)
{
	PiBool is_ok = FALSE;

	pi_lock_read_lock(&zip->write_lock, &zip->read_lock, LOCK_COUNT);

	_reload_zip(zip);

	if (zip->valid)
	{
		is_ok = pi_zip_item_get_info(zip->handle, name, info);
	}

	pi_lock_read_free(&zip->read_lock);

	return is_ok;
}

/* 获得文件或目录的大小，返回-1表示失败，失败可查错误编号 */
static int64 PI_API _get_size(VfsZip *zip, const wchar *name)
{
	int64 size = -1;

	pi_lock_read_lock(&zip->write_lock, &zip->read_lock, LOCK_COUNT);

	_reload_zip(zip);

	if (zip->valid)
	{
		size = pi_zip_item_get_size(zip->handle, name);
	}

	pi_lock_read_free(&zip->read_lock);

	return size;
}

/* 目录是否为空，返回是否成功，如成功，则is_empty存放目录是否为空，失败可查错误编号 */
static PiBool PI_API _dir_is_empty(VfsZip *zip, const wchar *name, PiBool *is_empty)
{
	PiBool exist = FALSE;

	pi_lock_read_lock(&zip->write_lock, &zip->read_lock, LOCK_COUNT);

	_reload_zip(zip);

	if (zip->valid)
	{
		exist = pi_zip_dir_is_empty(zip->handle, name, is_empty);
	}

	pi_lock_read_free(&zip->read_lock);

	return exist;
}

/* 打开目录，返回目录迭代器指针，NULL表示失败，失败可查错误编号 */
static VfsZipDir *PI_API _dir_open(VfsZip *zip, const wchar *name)
{
	VfsZipDir *dir = NULL;

	pi_lock_read_lock(&zip->write_lock, &zip->read_lock, LOCK_COUNT);

	_reload_zip(zip);

	if (zip->valid)
	{
		PiZipDir *handle = pi_zip_dir_open(zip->handle, name);

		if (handle)
		{
			dir = pi_new0(VfsZipDir, 1);
			pi_wstrcpy(dir->name, name, pi_wstrlen(name));
			dir->handle = handle;
			dir->index = 0;

			pi_lock_spin_lock(&zip->directories_lock, LOCK_COUNT);
			pi_vector_push(&zip->directories, dir);
			pi_lock_spin_free(&zip->directories_lock);
		}
	}

	pi_lock_read_free(&zip->read_lock);

	return dir;
}

/* 读取目录项，参数为目录迭代器指针，info为待填充的文件名称信息结构，返回是否结束 */
static PiBool PI_API _dir_read(VfsZip *zip, VfsZipDir *dir, PiFileNameInfo *info)
{
	PiBool is_ok = FALSE;

	pi_lock_read_lock(&zip->write_lock, &zip->read_lock, LOCK_COUNT);

	_reload_zip(zip);

	if (zip->valid)
	{
		is_ok = pi_zip_dir_read(dir->handle, dir->index, info);
		dir->index++;
	}

	pi_lock_read_free(&zip->read_lock);

	return is_ok;
}

/* 关闭目录 */
static PiBool PI_API _dir_close(VfsZip *zip, VfsZipDir *dir)
{
	uint count;
	pi_lock_spin_lock(&zip->directories_lock, LOCK_COUNT);
	count = pi_vector_remove_if(&zip->directories, pi_direct_equal, dir);
	pi_lock_spin_free(&zip->directories_lock);

	if (1 == count)
	{
		pi_free(dir);
		return TRUE;
	}

	pi_error_set(ERROR_TYPE_INTERNAL, 0, L"dir handle is invalid", __FILE__, __LINE__);
	return FALSE;
}

/* 文件操作 */
/* 取得文件的crc值，返回是否成功，如果成功则crc参数放置crc值，失败可查错误编号 */
static PiBool PI_API _file_get_crc32(VfsZip *zip, const wchar *name, uint32 *crc)
{
	PiBool is_ok = FALSE;

	pi_lock_read_lock(&zip->write_lock, &zip->read_lock, LOCK_COUNT);

	_reload_zip(zip);

	if (zip->valid)
	{
		is_ok = pi_zip_get_crc(zip->handle, name, crc);
	}

	pi_lock_read_free(&zip->read_lock);

	return is_ok;
}

/* 打开文件，返回文件指针，NULL为失败，失败可查错误编号 */
static VfsZipFile *PI_API _file_open(VfsZip *zip, const wchar *name, PiFileOpenMode mode)
{
	VfsZipFile *file = NULL;

	pi_lock_read_lock(&zip->write_lock, &zip->read_lock, LOCK_COUNT);

	_reload_zip(zip);

	if (zip->valid)
	{
		PiZipFile *handle = pi_zip_file_open(zip->handle, file->name);

		if (handle)
		{
			file = pi_new0(VfsZipFile, 1);
			pi_wstrcpy(file->name, name, pi_wstrlen(name));
			file->handle = handle;
			file->mode = mode;

			pi_lock_spin_lock(&zip->files_lock, LOCK_COUNT);
			pi_vector_push(&zip->files, file);
			pi_lock_spin_free(&zip->files_lock);
		}
	}

	pi_lock_read_free(&zip->read_lock);

	return file;
}

/* 关闭文件指针，返回是否成功，失败可查错误编号 */
static PiBool PI_API _file_close(VfsZip *zip, const void *handle)
{
	uint count;

	VfsZipFile *file = (VfsZipFile *)handle;

	pi_lock_spin_lock(&zip->files_lock, LOCK_COUNT);
	count = pi_vector_remove_if(&zip->files, pi_direct_equal, file);
	pi_lock_spin_free(&zip->files_lock);

	if (1 == count)
	{
		pi_free(file);
		return TRUE;
	}

	pi_error_set(ERROR_TYPE_INTERNAL, 0, L"file handle is invalid", __FILE__, __LINE__);
	return FALSE;
}

/* 从文件指针中获得文件大小，返回是否成功，失败可查错误编号 */
static PiBool PI_API _file_size(VfsZip *zip, const VfsZipFile *file, int64 *size)
{
	PiBool is_ok = FALSE;

	pi_lock_read_lock(&zip->write_lock, &zip->read_lock, LOCK_COUNT);

	_reload_zip(zip);

	if (zip->valid)
	{
		*size = pi_zip_file_get_size(file->handle);
	}

	pi_lock_read_free(&zip->read_lock);

	return is_ok;
}

/* 从文件指针中获得文件CRC32，返回是否成功，失败可查错误编号 */
static PiBool PI_API _file_crc32(VfsZip *zip, const VfsZipFile *file, uint32 *crc)
{
	pi_lock_read_lock(&zip->write_lock, &zip->read_lock, LOCK_COUNT);

	_reload_zip(zip);

	if (zip->valid)
	{
		*crc = pi_zip_file_get_crc(file->handle);
	}

	pi_lock_read_free(&zip->read_lock);

	return TRUE;
}

/* 读取文件，参数为文件指针，from_end为TRUE表示从尾部开始计算，位置为文件尾部减去offset的地方，返回实际读取的数据长度，-1表示失败，失败可查错误编号 */
static sint PI_API _file_read(VfsZip *zip, const VfsZipFile *file, int64 offset, PiBool from_end, char *data, uint len)
{
	sint ret = -1;
	pi_lock_read_lock(&zip->write_lock, &zip->read_lock, LOCK_COUNT);

	_reload_zip(zip);
	if (zip->valid)
	{
		ret = pi_zip_file_read(zip->handle, file->handle, offset, from_end, data, len);
	}

	pi_lock_read_free(&zip->read_lock);

	return ret;
}

/* 整理方法，参数为系统的启动后的毫秒时间 */
static void PI_API _collate(VfsZip *zip, int64 time)
{
	int64 t;
	if (!pi_lock_write(&zip->write_lock, &zip->read_lock, 1))
	{
		return;
	}

	t = pi_time_now() - zip->time;
	if (t < time || zip->valid)
	{
		pi_lock_write_free(&zip->write_lock);
		return;
	}

	pi_zip_close(zip->handle);
	zip->handle = NULL;
	zip->valid = FALSE;

	pi_lock_write_free(&zip->write_lock);
}

/* 打开函数 */
PiBool PI_API mod_open(PiVfsMod *mod, void *data, const wchar *name)
{
	VfsZip *zip = NULL;
	
	PiFileOpenMode mode = (PiFileOpenMode)data;

	PiZip *handle = pi_zip_open(name, mode);

	if (!handle)
	{
		return FALSE;
	}

	zip = pi_new0(VfsZip, 1);

	pi_wstrcpy(zip->name, name, pi_wstrlen(name));
	zip->mode = mode;

	zip->handle = handle;
	zip->valid = TRUE;
	zip->handle_lock = 0;

	zip->time = 0;

	pi_vector_init(&zip->files);
	zip->files_lock = 0;
	pi_vector_init(&zip->directories);
	zip->directories_lock = 0;

	zip->read_lock = 0;
	zip->write_lock = 0;

	mod->mod_data = zip;

	mod->get_info = _get_info;
	mod->get_size = _get_size;
	mod->set_time = NULL;
	mod->rename = NULL;
	mod->dir_create = NULL;
	mod->dir_delete = NULL;
	mod->dir_is_empty = _dir_is_empty;
	mod->dir_open = _dir_open;
	mod->dir_read = _dir_read;
	mod->dir_close = _dir_close;
	mod->file_get_crc32 = _file_get_crc32;
	mod->file_delete = NULL;
	mod->file_open = _file_open;
	mod->file_close = _file_close;
	mod->file_size = _file_size;
	mod->file_crc32 = _file_crc32;
	mod->file_read = _file_read;
	mod->file_write = NULL;
	mod->file_truncate = NULL;
	mod->collate = _collate;
	mod->thread_count = 0;
	return TRUE;
}

/* 关闭函数 */
PiBool PI_API mod_close(VfsZip *zip)
{
	//清空文件句柄
	pi_vector_foreach(&zip->files, pi_direct_free, NULL);
	pi_vector_clear(&zip->files, TRUE);

	//清空目录句柄
	pi_vector_foreach(&zip->directories, pi_direct_free, NULL);
	pi_vector_clear(&zip->directories, TRUE);

	if (zip->valid)
	{
		pi_zip_close(zip->handle);
	}

	pi_free(zip);

	return TRUE;
}
