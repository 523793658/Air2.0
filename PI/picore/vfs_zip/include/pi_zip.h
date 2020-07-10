/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 */

/**
 * ZIPѹ��������ģ�飬��֧��ZIP64 �ͼ���
 */

#ifndef __PI_ZIP_H__
#define __PI_ZIP_H__

#include <pi_lib.h>

typedef struct PiZip PiZip;
typedef struct PiZipDir PiZipDir;
typedef struct PiZipFile PiZipFile;

PI_BEGIN_DECLS

/**
 * ��һ��ZIP�ļ��������ؾ�������ʧ�ܣ��򷵻�NULL
 * @param name ZIP�ļ�������
 * @param mode �򿪷�ʽ
 * @returns ���ؾ�������ʧ�ܣ��򷵻�NULL
 */
PiZip *PI_API pi_zip_open(const wchar *name, PiFileOpenMode mode);

/**
 * �ر�zip�ļ����
 * @param zip zip���
 * @returns �����Ƿ�ɹ���ʧ�ܿɲ������
 */
PiBool PI_API pi_zip_close(PiZip *zip);

/**
 * ��ȡ�ļ���Ŀ¼����Ϣ
 * @param zip zip���
 * @param name �ļ���Ŀ¼������
 * @param info ����ļ���Ŀ¼����Ϣ
 * @returns �����Ƿ�ɹ���ʧ�ܿɲ������
 */
PiBool PI_API pi_zip_item_get_info(PiZip *zip, const wchar *name, PiFileInfo *info);

/**
 * ��ȡ�ļ���Ŀ¼����Ϣ
 * @param zip zip���
 * @param name �ļ���Ŀ¼������
 * @returns �����ļ���Ŀ¼�Ĵ�С������-1��ʾʧ�ܣ�ʧ�ܿɲ������
 */
int64 PI_API pi_zip_item_get_size(PiZip *zip, const wchar *name);

/**
 * �ж�һ��Ŀ¼�Ƿ�Ϊ��
 * @param zip zip���
 * @param name Ŀ¼������
 * @param is_empty ����Ŀ¼�Ƿ�Ϊ��
 * @returns ����Ŀ¼�Ƿ����
 */
PiBool PI_API pi_zip_dir_is_empty(PiZip *zip, const wchar *name, PiBool *is_empty);

/**
 * ��Ŀ¼���
 * @param zip zip���
 * @param name Ŀ¼������
 * @returns ����Ŀ¼�����NULLΪʧ�ܣ�ʧ�ܿɲ������
 */
PiZipDir *PI_API pi_zip_dir_open(PiZip *zip, const wchar *name);

/**
 * ��ȡĿ¼������
 * @param dir Ŀ¼�ľ��
 * @param index ��������
 * @param info �������Ϣ
 * @returns �����Ƿ�ɹ���ʧ�ܿɲ������
 */
PiBool PI_API pi_zip_dir_read(PiZipDir *dir, uint32 index, PiFileNameInfo *info);

/**
 * ͨ���ļ�����ȡcrcֵ
 * @param zip zip���
 * @param name �ļ�������
 * @param crc �����crcֵ
 * @returns �����ļ��Ƿ����
 */
PiBool PI_API pi_zip_get_crc(PiZip *zip, const wchar *name, uint32 *crc);

/**
 * ���ļ����
 * @param zip zip���
 * @param name �ļ�������
 * @returns �ļ������NULLΪʧ�ܣ�ʧ�ܿɲ������
 */
PiZipFile *PI_API pi_zip_file_open(PiZip *zip, const wchar *name);

/**
 * ��ȡ�ļ���С
 * @param file �ļ����
 * @returns �ļ���С
 */
uint32 PI_API pi_zip_file_get_size(PiZipFile *file);

/**
 * ��ȡ�ļ������ȡCRCֵ
 * @param file �ļ����
 * @returns crcֵ
 */
uint32 PI_API pi_zip_file_get_crc(PiZipFile *file);

/**
 * ��ȡ�ļ�����
 * @param zip zip���
 * @param file �ļ����
 * @param offset ƫ����
 * @param from_end �Ƿ���ļ�β������ƫ��
 * @param data �������
 * @param len ��������С
 * @returns ʵ�ʶ�ȡ�����ݳ��ȣ�-1��ʾʧ�ܣ�ʧ�ܿɲ������
 */
sint PI_API pi_zip_file_read(PiZip *zip, const PiZipFile *file, int64 offset, PiBool from_end, char *data, uint len);

PI_END_DECLS

#endif /* __PI_ZIP_H__ */
