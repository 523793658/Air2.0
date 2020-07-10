#ifndef INCLUDE_COLORGRADING_H
#define INCLUDE_COLORGRADING_H

#include <renderer.h>
#include <texture.h>

/**
 * Color Grading: ��ɫӳ���
 * ���ã�ʹ��lut��look up table����Ϊ��ɫӳ�䣬�����һ��Ч��
 * ���룺Texture����������
 * �����Texture������color gradingӳ��֮��ĳ�������
 * ӳ���3D Texture����ʾRGB�ռ��ӳ���ϵ
 */

const static char* RS_COLOR_GRADING_VS = "default.vs";
const static char* RS_COLOR_GRADING_FS = "color_grading.fs";

PI_BEGIN_DECLS

PiRenderer* PI_API pi_colorgrading_new();

void PI_API pi_colorgrading_deploy(PiRenderer* renderer, char* input_name, char* output_name);

void PI_API pi_colorgrading_free(PiRenderer* renderer);

/* lut��look up table����ɫ�ռ�ӳ���һ����16*16*16����ά������ */
void PI_API pi_colorgrading_set_lut(PiRenderer *renderer, PiTexture *lut);

PI_END_DECLS

#endif /* INCLUDE_COLORGRADING_H */
