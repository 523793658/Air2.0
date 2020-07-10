#ifndef INCLUDE_RENDERINFO_H
#define INCLUDE_RENDERINFO_H

/**
 * 渲染反馈信息，上下文
 */

#include <pi_lib.h>

PI_BEGIN_DECLS
	
void PI_API pi_renderinfo_set_current_renderer_name(const char *name);
const char* PI_API pi_renderinfo_get_current_renderer_name();

uint PI_API pi_renderinfo_get_texture_num(void);

uint PI_API pi_renderinfo_get_target_num(void);

uint PI_API pi_renderinfo_get_vb_num(void);

uint PI_API pi_renderinfo_get_ib_num(void);

uint PI_API pi_renderinfo_get_vs_num(void);

uint PI_API pi_renderinfo_get_fs_num(void);

uint PI_API pi_renderinfo_get_gpuprogram_num(void);

uint PI_API pi_renderinfo_get_view_num(void);

uint PI_API pi_renderinfo_get_face_num(void);

uint PI_API pi_renderinfo_get_vertex_num(void);

uint PI_API pi_renderinfo_get_entity_num(void);

/* 每帧切换信息 */
uint PI_API pi_renderinfo_get_vb_change_num(void);

uint PI_API pi_renderinfo_get_ib_change_num(void);

uint PI_API pi_renderinfo_get_texture_change_num(void);

uint PI_API pi_renderinfo_get_gpuprogram_change_num(void);

uint PI_API pi_renderinfo_get_target_change_num(void);

/* set */
void PI_API pi_renderinfo_add_texture_num(sint num);

void PI_API pi_renderinfo_add_particle_num(sint num);

uint PI_API pi_renderinfo_get_particle_num(void);

void PI_API pi_renderinfo_add_target_num(sint num);

void PI_API pi_renderinfo_add_vb_num(sint num);

void PI_API pi_renderinfo_add_ib_num(sint num);

void PI_API pi_renderinfo_add_vs_num(sint num);

void PI_API pi_renderinfo_add_fs_num(sint num);

void PI_API pi_renderinfo_add_gpuprogram_num(sint num);

void PI_API pi_renderinfo_add_view_num(sint num);

void PI_API pi_renderinfo_add_face_num(sint num);

void PI_API pi_renderinfo_add_vertex_num(sint num);

void PI_API pi_renderinfo_add_entity_num(sint num);

void PI_API pi_renderinfo_add_vb_change_num(sint num);

void PI_API pi_renderinfo_add_ib_change_num(sint num);

void PI_API pi_renderinfo_add_texture_change_num(sint num);

void PI_API pi_renderinfo_add_gpuprogram_change_num(sint num);

void PI_API pi_renderinfo_add_target_change_num(sint num);

/* 重置每帧的渲染信息 */
void PI_API pi_renderinfo_reset(void);

PI_END_DECLS

#endif /* INCLUDE_RENDERINFO_H */