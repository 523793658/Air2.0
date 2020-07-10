#ifndef INCLUDE_FLOOD_CONTROOLER_H
#define INCLUDE_FLOOD_CONTROOLER_H

#include "app_controller.h"

typedef enum
{
    PRIORITY_ONE,   /* 第一层优先级 */
    PRIORITY_TWO,   /* 第二层优先级 */
    PRIORITY_THREE, /* 第三层优先级 */
    PRIORITY_FOUR,  /* 第四层优先级 */
    PRIORITY_COUNT  /* 优先级总数  */
} FloodPriority;

typedef struct
{
    int count;         /* 执行次数 */
    float curr_time;   /* 当前时间 */
    float duration;    /* 周期时间 */
    float start_scale; /* 开始亮度 */
    float end_scale;   /* 结束亮度 */
    PiBool is_loop;    /* 是否循环 */
    PiBool is_flash;   /* 是否闪烁 */
    PiVector3 start_color;  /* 开始颜色 */
    PiVector3 end_color;    /* 结束颜色 */
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
 * 创建泛光控制器
 */
PiController *PI_API pi_flood_new();

/**
 * 释放控制器
 */
void PI_API pi_flood_free(PiController *c);

/**
 * 设置控制器材质参数
 * @param c 控制器
 * @param flood_tex 材质贴图
 * @param u uv
 * @param v uv
 */
PiBool PI_API pi_flood_set_map(PiController *c, PiTexture* flood_tex, float u, float v);

/**
 * 开始泛光
 * @param c 控制器
 * @param start_x 开始颜色的r
 * @param start_y 开始颜色的g
 * @param start_z 开始颜色的b
 * @param end_x 结束颜色的r
 * @param end_y 结束颜色的g
 * @param end_z 结束颜色的b
 * @param start_scale 开始亮度
 * @param end_scale 结束亮度
 * @param duration 周期时间
 * @param start_time 开始时间
 * @param count 执行次数
 * @param loop 是否循环
 * @param flash 是否闪烁
 * @param priority 优先级层数
 */
PiBool PI_API pi_flood_set_play(PiController *c, float start_x, float start_y, float start_z, float end_x, float end_y,
                                float end_z, float start_scale, float end_scale, float duration, float start_time, 
                                int count, PiBool loop, PiBool flash, FloodPriority priority);

/**
 * 停止泛光
 * @param c 控制器
 * @param priority 优先级层数
 */
PiBool PI_API pi_flood_stop(PiController *c, FloodPriority priority);

PI_END_DECLS

#endif /* INCLUDE_FLOOD_CONTROOLER_H */