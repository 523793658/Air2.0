#ifndef INCLUDE_COMPONENT_H
#define INCLUDE_COMPONENT_H

#include "rectangle.h"
#include "texture.h"
#include <renderview.h>
#include <rendertarget.h>

/**
 * ���ļ�������UIϵͳ�е�ԭʼ���
 * UIϵͳ�ĺ���ģ��,���о���ؼ����ɴ�ģ����չ
 * �����ṩUIϵͳ���������/���ƻ��Ƶȳ�ȥ������Ʒ�������Ļ�������
 * ��Ҫע�����,��ģ��Ĳ��ַ������ܻᱻ����Ŀؼ�ģ��ӹ�,�����������,������ô�ģ���
 * ���ֽӿڽ������쳣,����ο���Ӧ�ؼ�ģ���ڵ�˵��
 */

#define COMPONENT_LAYER_MAX 100
#define COMPONENT_LAYER_MIM 0

typedef struct PiComponent PiComponent;
typedef void (*ComponentDrawFunc)(PiComponent *component, void *impl);
typedef void (*ComponentUpdateFunc)(PiComponent *component, float tpf);

typedef enum
{
	EWT_PANEL = 0,
	EWT_LABEL
} EWidgetsType;

struct PiComponent
{
	EWidgetsType type;

	uint width;
	uint hight;
	PiRectangle bounds;
	float scale[2];
	sint layer;

	PiBool is_visible;
	PiBool is_independent;
	PiBool is_translucent;

	PiComponent *parent;
	PiVector *children;

	PiRectangle global_bounds;
	float global_scale[2];
	PiRectangle reprint_bounds;
	float global_layer;		//ʵ��DepthTest�Ż�ʱ�ı����ֶ�

	//Masks TODO:�ϲ��ֶ�
	PiBool is_bounds_update;
	PiBool is_reprint;
	PiBool is_internal_reprint;
	PiBool is_layer_update;

	PiRenderView *independent_view_buffer;
	PiTexture *independent_view_texture;

	ComponentDrawFunc draw_func;
	ComponentUpdateFunc update_func;
	void *impl;
};

PI_BEGIN_DECLS

/**
 * ����һ��UI���,�ɾ���ؼ�ģ���ʵ�ָ���
 * @param type �ؼ�����
 * @param draw_func �ؼ��Ļ��Ʒ���,ΪNULLʱ�Զ�ʹ��Ĭ�ϵĿպ������
 * @param update_func �ؼ��ĸ��·���,ΪNULLʱ�Զ�ʹ��Ĭ�ϵĿպ������
 * @param impl �ؼ��ľ���ʵ��
 * @return ���ָ��
 */
PiComponent *PI_API pi_component_new(EWidgetsType type, ComponentDrawFunc draw_func, ComponentUpdateFunc update_func, void *impl);

/**
 * �ͷ�һ��UI���,�ɾ���ؼ�ģ���ʵ�ָ���,
 * ע��:����ʱ�Ĵ�����ᱻ�˺����ͷ�
 * @param component ��Ҫ�ͷŵ����ָ��
 */
void PI_API pi_component_delete(PiComponent *component);

/**
 * ��һ��ָ��������Ϊĳ����������
 * �ӿؼ�������/��ʾ/�任�Ƚ��Ը��ؼ�Ϊ��Ը����ϵ
 * Ĭ�������,�ӿؼ��޷��ڸ��ؼ��Ļ��Ʒ�Χ����ʾ(һЩ�ض��Ĳ��ֹ�����������ӿؼ���С�ı丸�ؼ�,��һ���̶��ƹ�������)
 * �ٴ����һ���Ѿ������Ϊ�ӿؼ������ʱ�����´��������ǰ�ĸ�����ϱ��Զ��Ƴ�
 * @param parent �����ָ��
 * @param child �����ָ��
 * @param layout ���ֹ������,ע��˲������ڳɹ�remove�˿ؼ�ʱ���Զ��ͷ�
 */
void PI_API pi_component_attach_child(PiComponent *parent, PiComponent *child);

/**
 * ��һ��ָ��������丸������Ƴ�
 * ��������������,�����κ�Ч��
 * �˷������Զ��ͷ�addʱ��layout������
 * @param child �����ָ��
 */
void PI_API pi_component_detach_from_parent(PiComponent *child);

/**
 * ָ������ĳ���
 * @param component ���ָ��
 * @param width �������
 * @param height ������
 */
void PI_API pi_component_set_size(PiComponent *component, uint width, uint height);

/**
 * ָ�������λ��
 * @param component ���ָ��
 * @param x ���λ��X����
 * @param y ���λ��Y����
 */
void PI_API pi_component_set_location(PiComponent *component, sint x, sint y);

void PI_API pi_component_get_size(PiComponent *component, uint size[2]);

void PI_API pi_component_get_location(PiComponent *component, sint location[2]);

/**
 * ָ�����������
 * ע��:�˽ӿ�������pi_component_set_size,ǰ���ǻ���ʱ��ԭʼ��С,�Ҹı�ʱ��Ӱ���ӿؼ�,�˽ӿڲ�����֤���ؾ���,
 * ������ɵķŴ�ĳ���ؼ����ܵ��¿ؼ����־��Ȳ���ľ��,�����Ÿ��ڵ�ʱ�ӽڵ�Ҳ����Ӧ�������ű�����Ե������������
 * �ڶ������������,�˽ӿ��ٶ�ԶԶ����ǰ��
 *
 * TODO:ScaleĿǰ����int��floatת����ľ��ȶ�ʧ����,�ᵼ�¶���������ĸ��ؼ�scale���ӿؼ��˶�ʱ�Ļ��Ʒ����������������������
 * �������Ϊ�̶�����������֮������пؼ���һ��ͳһ������ϵ(��ǿ������Ϊ1),��������Ҫ������ڴ����ݴ洢�ͼ���
 * @param component ���ָ��
 * @param x �������X����
 * @param y �������Y����
 */
void PI_API pi_component_set_scale(PiComponent *component, float x, float y);

/**
 * ָ������Ļ��Ʋ㼶
 * @param component ���ָ��
 * @param layer ������Ʋ㼶,ͬ������ĺ�����LayerԽ�������������Ƶ�Խ��ǰ��λ��,Ĭ��Ϊ0,����Ϊ����
 */
void PI_API pi_component_set_layer(PiComponent *component, sint layer);

/**
 * ָ������Ƿ�ɼ�
 * ��������������,�����������Ҳ���޷���ʾ
 * @param component ���ָ��
 * @param b ����Ƿ�ɼ�
 */
void PI_API pi_component_set_visible(PiComponent *component, PiBool b);

/**
 * ָ������Ƿ��������
 * ���һ�������ָ��Ϊ��������,��ô�������Լ������ӿؼ��Ļ��ƽ�����������Ļ�������
 * �����ڲ���Ҫ���ػ������,�����󲿷��ػ����󱻴���ʱ��Щ���������Ϊһ����һ���,����ʱ�������һ��DrawCall
 * ����ʹ�ô˷���������Ч��ԼЧ��,����������ڲ��ṹ�̶����䵫��Ϊһ�����屻Ƶ�����µ����
 * ע��:Ŀǰʵ���и����(����ΪNULL),������ô˽ӿ�,һ����Ϊ������������
 * @param component ���ָ��
 * @param b ����Ƿ�����������
 */
void PI_API pi_component_set_independent(PiComponent *component, PiBool b);

/**
 * ָ������Ƿ�Ϊ��͸�����
 * TODO:��ʱδʵ��
 * ��������͸��,��ô����ʱ��ʹ��DepthBuffer���Ż������ٶȺ�����������㷨,���Ի�ø��ߵ�Ч��
 * ���������,��͸���ؼ��Ļ��ƽ����ܳ����쳣
 * @param component ���ָ��
 * @param b ����Ƿ��͸��,Ĭ��ΪFalse
 */
void PI_API pi_component_set_translucent(PiComponent *component, PiBool b);

/************************************************************************/
/*   ����Ϊϵͳ�ڲ����ýӿ�,��������������,����ʱ���û����벻Ӧ�õ������½ӿ�       */
/************************************************************************/

/**
 * ǿ���ػ�ָ�����
 * �󲿷�ʱ��˷���������Ҫʱ�Զ�����,������ڽ���ĳЩ����������ʾ�쳣ʱ,�ɳ��Ե��ô˷���
 * @param component ���ָ��
 */
void PI_API pi_component_reprint(PiComponent *component);

/**
 * �������
 */
void PI_API pi_component_draw(PiComponent *component);

/**
 * �������ɵĻ���
 */
void PI_API pi_component_finish_draw(PiComponent *component);

/**
 * �������
 */
void PI_API pi_component_update(PiComponent *component, float tpf);

/**
 * ��ȡ��ǰ֡���ػ�����
 */
void PI_API pi_component_get_reprint_components(PiComponent *component, PiVector *result_buffer, PiVector *tmp_stack, PiVector *flip_stack);

/**
 * �������
 */
void PI_API pi_component_sort_layer(PiVector *components);

PI_END_DECLS

#endif /* INCLUDE_COMPONENT_H */
