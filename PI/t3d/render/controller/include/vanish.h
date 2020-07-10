#ifndef INCLUDE_VANIASH_H
#define INCLUDE_VANIASH_H

#include <controller.h>

PI_BEGIN_DECLS

/**
 * ����
 */
PiController *PI_API pi_vanish_new();

/**
 * ������ز���
 * @param vanish_time ��ɢ��ʱ��
 * @param vanish_gap ��ɢ��Ե��϶��Ҳ���Ǳ�Ե����alpha�ļ��������ڵ�����Ϊ��ɢ��Ե
 */
void PI_API pi_vanish_set_parameter(PiController *c, float vanish_time, float vanish_gap, PiBool end_visible);
/*
*����Ч��
*/
void PI_API pi_vanish_reset(PiController *c, void *obj);
/*
 * �ͷ�
 */
void PI_API pi_vanish_free(PiController *c);

PI_END_DECLS

#endif /* INCLUDE_VANIASH_H */
