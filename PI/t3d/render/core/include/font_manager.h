#ifndef INCLUDE_FONT_MANAGER_H
#define INCLUDE_FONT_MANAGER_H

#include "texture_atlas.h"

/* Windowsϵͳ������������Ӣ������ */
typedef enum
{
	FFT_INVALID = -1,			/* ��Ч�� */
	FFT_MICROSOFT_YA_HEI,		/* ΢���ź� */
	FFT_MICROSOFT_YA_HEI_UI,	/* ΢���ź�UI */
	FFT_SIM_HEI,				/* ���� */
	FFT_KAI_TI,					/* ���� */
	FFT_SIM_SUN,				/* ���� */
	FFT_N_SIM_SUN,				/* ������ */
	FFT_FANG_SONG,				/* ���� */
	FFT_Li_Su,					/* ���� */
	FFT_You_Yuan,				/* ��Բ */
	FFT_NUM
} PiFontFamilyType;

typedef enum
{
	FFS_INVALID = -1,			/* ��Ч�� */
	FFS_REGULAR,				/* ���� */
	FFS_BOLD,					/* ���� */
	FFS_ITALIC,					/* б�� */
	FFS_BLACK,					/* ���� */
	FFS_SEMIBOLD,				/* ����� */
	FFS_MEDIUM,					/* �е� */
	FFS_LIGHT,					/* ϸ�� */
	FFS_EXTRA_LIGHT,			/* ��ϸ */
	FFS_NUM
} PiFontFaceStyle;

typedef enum
{
	OT_NONE,					/* ����������׼������ */
	OT_LINE,					/* ������������ֻ��������������Ҳ�����ڲ��ǿգ����Ը������ú�� */
	OT_INNER,					/* ������������ */
	OT_OUTER					/* ����������չ */
} PiOutlineType;

typedef struct PiFontManager PiFontManager;

typedef struct PiFontFamily PiFontFamily;

typedef struct PiFontFace PiFontFace;

typedef struct PiFontFaceSize PiFontFaceSize;

/* �ַ����������� */
typedef struct
{
	wchar charcode;			/* �ַ��� */
	uint advance_x;			/* �ַ����Ű��� */

	/* ����ͼ����Ի��ߵ����꣬���������ַ��������������sint���ͣ�����Ӽ���ĸ������ڴ�������ʱ��Ҫ���д�������ת�� */
	sint left;				/* �ַ����������߽�����ַ�ԭ��ľ��룬��λ���� */
	sint right;				/* �ַ���������ұ߽�����ַ�ԭ��ľ��룬��λ���� */
	sint top;				/* �ַ���������ϱ߽�����ַ����ߵľ��룬��λ���� */
	sint bottom;			/* �ַ���������±߽�����ַ�����ľ��룬��λ���� */

	/* ����ͼ������������꣬���������ַ���uv���꣬����ֱ�Ӳ�����float���ͣ����������ʱ��Ͳ���Ҫ����ǿ������ת�� */
	float s0;
	float t0;
	float s1;
	float t1;
} PiCharGlyph;

PI_BEGIN_DECLS

/**
 * �������������
 * @param horz_resolution ��Ļ��ˮƽDPI��WindowsĬ��96
 * @param vert_resolution ��Ļ�Ĵ�ֱDPI��WindowsĬ��96
 * @returns ������������
 */
PiFontManager *PI_API pi_font_manager_new(uint horz_resolution, uint vert_resolution);

/**
 * �������������
 * @param manager ������������
 */
void PI_API pi_font_manager_delete(PiFontManager *manager);

/**
 * ������������
 * @param manager ������������
 * @param data ��������
 * @param data_size �������ݵĴ�С
 * @param force ǿ�Ƽ��أ�Ĭ��FALSE
 */
void PI_API pi_font_manager_load_data(PiFontManager *manager, const byte *data, uint data_size, PiBool force);

/**
 * ��ȡfont-family
 * @param manager ������������
 * @param type font-family�����ͣ�����FFT_MICROSOFT_YA_HEI
 * @returns font-family���
 */
PiFontFamily *PI_API pi_font_family_get(PiFontManager *manager, PiFontFamilyType type);

/**
 * ����font-family
 * @param family font-family���
 */
void PI_API pi_font_family_delete(PiFontFamily *family);

/**
 * ��ȡfont-face
 * @param family font-family���
 * @param style face�ķ�񣬱���"FFS_ITALIC"��ʾб��
 * @returns font-face���
 */
PiFontFace *PI_API pi_font_face_get(PiFontFamily *family, PiFontFaceStyle style);

/**
 * ����font-face
 * @param face font-face���
 */
void PI_API pi_font_face_delete(PiFontFace *face);

/**
 * ����face-size
 * @param face font-face���
 * @param point_size �ַ��İ���
 * @param outline_type ����������
 * @param outline_thickness �������
 * @param bold �Ƿ�Ӵ�
 * @param strength �Ӵֺ�ȣ���λ���أ�26.6���ظ�ʽ
 * @param italic �Ƿ���б
 * @param lean ��б�ȣ�ƫ�Ƶľ������ַ��߶ȵı�ֵ��16.16�Ķ�������ʽ
 * @returns face-size���
 */
PiFontFaceSize *PI_API pi_font_face_size_new(PiFontFace *face, sint point_size, PiOutlineType outline_type, sint outline_thickness, PiBool bold, sint strength, PiBool italic, sint lean);

/**
 * ����face-size
 * @param size face-size���
 */
void PI_API pi_font_face_size_delete(PiFontFaceSize *size);

/**
 * �����ַ�������
 * @param size face-size���
 * @param charcode �ַ�
 * @returns ���ξ��
 */
PiCharGlyph *PI_API pi_font_face_size_load_char_glyph(PiFontFaceSize *size, wchar charcode);

/**
 * ��ȡface-size��������
 * @param size face-size���
 * @returns ����������
 */
PiTextureAtlasData *PI_API pi_font_face_size_get_glyphs_texture(PiFontFaceSize *size);

PI_END_DECLS

#endif /* INCLUDE_FONT_MANAGER_H */
