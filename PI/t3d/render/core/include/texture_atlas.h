#ifndef INCLUDE_TEXTURE_ATLAS_H
#define INCLUDE_TEXTURE_ATLAS_H

#include "material.h"

typedef struct PiTextureAtlas PiTextureAtlas;		/* ������ͼ�� */

typedef struct
{
	SamplerState atlas_sampler;						/* ������ͼ�������������������nearest */
	float atlas_size[2];							/* ������ͼ��������Ŀ�� */
} PiTextureAtlasData;

PI_BEGIN_DECLS

/**
 * ����������ͼ��
 * @returns ������ͼ�����
 */
PiTextureAtlas *PI_API pi_texture_atlas_new();

/**
 * ����������ͼ��
 * @param atlas ������ͼ�����
 */
void PI_API pi_texture_atlas_delete(PiTextureAtlas *atlas);

/**
 * ��������ͼ�������һ���ֿ�
 * @param atlas ������ͼ�����
 * @param bitmap �ֿ������
 * @param width �ֿ�Ŀ��
 * @param height �ֿ�ĸ߶�
 * @param offset ���طֿ���������ͼ����ƫ����
 */
void PI_API pi_texture_atlas_add_tile(PiTextureAtlas *atlas, byte *bitmap, uint width, uint height, uint offset[2]);

/**
 * ��ȡ������ͼ������Ⱦ����
 * @returns ������ͼ������Ⱦ���ݵľ��
 */
PiTextureAtlasData *PI_API pi_texture_atlas_get_data(PiTextureAtlas *atlas);

PI_END_DECLS

#endif /* INCLUDE_TEXTURE_ATLAS_H */
