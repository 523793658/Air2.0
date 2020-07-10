#ifndef INCLUDE_UI_RENDERER_H
#define INCLUDE_UI_RENDERER_H

#include <renderer.h>

/**
 * ���ļ������˻���UI����Ⱦ��
 */

PI_BEGIN_DECLS

/**
 * ����UI��Ⱦ��
 * @return ��Ⱦ��ָ��
 */
PiRenderer *PI_API pi_ui_renderer_new();

/**
 * ����UI��Ⱦ��
 * @param renderer ��Ⱦ��ָ��
 * @param target_name ��ȾĿ����
 * @param component_list_name ����б���,ע��:�б��ڱ�����PiComponent,���丸�������ΪNull����ֱ�ӱ�����
 */
void PI_API pi_ui_renderer_deploy(PiRenderer *renderer, char *target_name, char *component_list_name);

/**
 * �ͷ�UI��Ⱦ��
 * @param renderer ��Ⱦ��ָ��
 */
void PI_API pi_ui_renderer_free(PiRenderer *renderer);

PI_END_DECLS

#endif /* INCLUDE_UI_RENDERER_H */