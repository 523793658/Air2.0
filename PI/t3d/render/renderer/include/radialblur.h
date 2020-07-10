#ifndef INCLUDE_RADIALBLUR_H
#define INCLUDE_RADIALBLUR_H

#include <renderer.h>

const static char* RS_RADIALBLUR_VS = "default.vs";
const static char* RS_RADIALBLUR_FS = "radialblur.fs";

PI_BEGIN_DECLS

PiRenderer* PI_API pi_radialblur_new();

void PI_API pi_radialblur_deploy(PiRenderer* renderer, uint width, uint height, char* input_name, char* output_name);

void PI_API pi_radialblur_free(PiRenderer* renderer);

/* �������ã���Χ��[0.0, 1.0]��Ĭ����1.0��0��ʾû��Ч����1��ʾ���Ч�� */
void PI_API pi_radialblur_set_progress(PiRenderer* renderer, float progress);

/* �����������ã�Ĭ����1��ֵԽ���ʾ�����ĵ��뵱ǰ���ص�ԽԶ */
void PI_API pi_radialblur_set_sample_dist(PiRenderer* renderer, float sample_dist);

/* ���ȣ�Ĭ����2.2��Խ���ʾԽ�� */
void PI_API pi_radialblur_set_sample_strength(PiRenderer* renderer, float sample_strength);

PI_END_DECLS

#endif /* INCLUDE_RADIALBLUR_H */