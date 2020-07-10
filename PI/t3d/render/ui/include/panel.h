#ifndef INCLUDE_PANEL_H
#define INCLUDE_PANEL_H

#include "component.h"
#include "texture.h"

/**
 * ���ļ�������������ʾͼƬ��Panel�ؼ�
 * �˿ؼ���pi_panel_set_translucent�ӿ������pi_component_set_translucent,�������κ�ʱ��Ӧ������
 */
PI_BEGIN_DECLS

/**
 * ����Panel�ؼ�
 * @return �ؼ�ָ��
 */
PiComponent *PI_API pi_panel_new();

/**
 * �ͷſؼ�
 * ע�������ٵ���component��free����
 * @param panel ��Ҫ�ͷŵ�Panel�ؼ�ָ��
 */
void PI_API pi_panel_delete(PiComponent *panel);

/**
 * ���ÿؼ���ʾ����ɫ��͸����
 * ע��:����ɫ����������ʱ������˵ķ�ʽ��������
 * @param panel �ؼ�ָ��
 * @param r ��ɫRͨ��,��Χ0~1,Ĭ��Ϊ1
 * @param g ��ɫGͨ��,��Χ0~1,Ĭ��Ϊ1
 * @param b ��ɫBͨ��,��Χ0~1,Ĭ��Ϊ1
 * @param a ��ɫAͨ��,��Χ0~1,ֱ��Ӱ��ؼ�������͸����,Ĭ��Ϊ1����͸��
 */
void PI_API pi_panel_set_color(PiComponent *panel, float r, float g, float b, float a);

/**
 * ���ÿؼ����Ƶ�ʹ�õ�����
 * @param panel �ؼ�ָ��
 * @param texture ����ָ��,NULL��ʾ�ؼ�ʹ�ô���ɫ����
 */
void PI_API pi_panel_set_texture(PiComponent *panel, PiTexture *texture);

/**
 * ���ÿؼ����������UV����,��u��v��Ϊ0ʱ��ʾ�ر�uv����
 * ע��:����UV������ǿ�ƹر�����֡����
 * @param panel �ؼ�ָ��
 * @param u UV����u�����ٶ�
 * @param v UV����v�����ٶ�
 */
void PI_API pi_panel_set_texture_uv_anim(PiComponent *panel, float u, float v);

/**
 * ���ÿؼ����������Tile����֡����,��tile_x��tile_y��������1ʱ,��ʾ�ر�����֡����
 * ע��:��������֡������ǿ�ƹر�UV����
 * @param panel �ؼ�ָ��
 * @param tile_x ˮƽ����Ķ���֡����
 * @param tile_y ��ֱ����Ķ���֡����
 * @param frame_time ÿ����֡�ĳ���ʱ��,Ҳ������ '1 / fps' ����
 * @param tile_count �������ܼƲ���Tile֡����,��һ��3*3Tile������,����Ϊ5,�򶯻�ֻ��ʹ��ǰ5֡��������ѭ��,�˲���Ϊ0ʱ���Զ�����Ϊtile_x * tile_y
 * @param is_blend �Ƿ�����֡���Ϲ���,�����ɵõ���ƽ����Ч��,���ή������
 */
void PI_API pi_panel_set_texture_tile_anim(PiComponent *panel, uint tile_x, uint tile_y, float frame_time, uint tile_count, PiBool is_blend);

/**
 * ���ÿؼ��Ƿ����ð�͸������,�������ʱ���ص�Alpha��ǿ����Ϊ1
 * @param panel �ؼ�ָ��
 * @param b �Ƿ��͸��,Ĭ��ΪFalse
 */
void PI_API pi_panel_set_translucent(PiComponent *panel, PiBool b);

PI_END_DECLS

#endif /* INCLUDE_PANEL_H */
