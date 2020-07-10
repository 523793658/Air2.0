#ifndef INCLUDE_FLOOD_CONTROOLER_H
#define INCLUDE_FLOOD_CONTROOLER_H

#include "app_controller.h"

typedef enum
{
    PRIORITY_ONE,   /* ��һ�����ȼ� */
    PRIORITY_TWO,   /* �ڶ������ȼ� */
    PRIORITY_THREE, /* ���������ȼ� */
    PRIORITY_FOUR,  /* ���Ĳ����ȼ� */
    PRIORITY_COUNT  /* ���ȼ�����  */
} FloodPriority;

typedef struct
{
    int count;         /* ִ�д��� */
    float curr_time;   /* ��ǰʱ�� */
    float duration;    /* ����ʱ�� */
    float start_scale; /* ��ʼ���� */
    float end_scale;   /* �������� */
    PiBool is_loop;    /* �Ƿ�ѭ�� */
    PiBool is_flash;   /* �Ƿ���˸ */
    PiVector3 start_color;  /* ��ʼ��ɫ */
    PiVector3 end_color;    /* ������ɫ */
} FloodParam;

typedef struct
{
    int weight;
    float flood_scale;
    PiBool show_flag;
    PiVector3 flood_color;
    FloodParam *params[PRIORITY_COUNT];

    PiBool flood_map;
    SamplerState sampler;
    PiTexture* flood_tex;
    float uv[2];

    char *FLOOD;
    char *FLOOD_TEX;
    char *U_FloodScale;
    char *U_FloodColor;
    char *U_FLoodUVVector;
    char *U_FloodMap;
} Flood;

PI_BEGIN_DECLS

/**
 * �������������
 */
PiController *PI_API pi_flood_new();

/**
 * �ͷſ�����
 */
void PI_API pi_flood_free(PiController *c);

/**
 * ���ÿ��������ʲ���
 * @param c ������
 * @param flood_tex ������ͼ
 * @param u uv
 * @param v uv
 */
PiBool PI_API pi_flood_set_map(PiController *c, PiTexture* flood_tex, float u, float v);

/**
 * ��ʼ����
 * @param c ������
 * @param start_x ��ʼ��ɫ��r
 * @param start_y ��ʼ��ɫ��g
 * @param start_z ��ʼ��ɫ��b
 * @param end_x ������ɫ��r
 * @param end_y ������ɫ��g
 * @param end_z ������ɫ��b
 * @param start_scale ��ʼ����
 * @param end_scale ��������
 * @param duration ����ʱ��
 * @param start_time ��ʼʱ��
 * @param count ִ�д���
 * @param loop �Ƿ�ѭ��
 * @param flash �Ƿ���˸
 * @param priority ���ȼ�����
 */
PiBool PI_API pi_flood_set_play(PiController *c, float start_x, float start_y, float start_z, float end_x, float end_y,
                                float end_z, float start_scale, float end_scale, float duration, float start_time, 
                                int count, PiBool loop, PiBool flash, FloodPriority priority);

/**
 * ֹͣ����
 * @param c ������
 * @param priority ���ȼ�����
 */
PiBool PI_API pi_flood_stop(PiController *c, FloodPriority priority);

PI_END_DECLS

#endif /* INCLUDE_FLOOD_CONTROOLER_H */