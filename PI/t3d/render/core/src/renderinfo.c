#include <renderinfo.h>

typedef struct  
{
	/* 当前运行的渲染器的名字 */
	const char *curr_renderer_name;

	/* 渲染资源相关 */
	volatile sint tex_num;		/* 纹理数量 */
	volatile sint ib_num;		/* 索引的数量 */
	volatile sint vb_num;		/* 顶点流的数量 */
	volatile sint target_num;	/* RenderTarget的数量 */
	volatile sint view_num;		/* RenderView的数量 */
	volatile sint vs_num;		/* vs的数量 */
	volatile sint fs_num;		/* fs的数量 */
	volatile sint gpu_program_num;	/* GPU program 的数量 */

	/* 每帧渲染相关 */
	volatile sint face_num;		/* 该帧渲染的面数 */
	volatile sint vertex_num;	/* 该帧渲染的顶点数 */
	volatile sint entity_num;	/* 该帧渲染的DrawCall数 */
	volatile sint particle_num; /* 该帧渲染的粒子数
	
	/* 每帧切换信息 */
	volatile sint vb_change_num;	/* VB切换信息 */
	volatile sint ib_change_num;	/* IB切换信息 */
	volatile sint tex_change_num;	/* TEX切换信息 */
	volatile sint gpuprogram_change_num;	/* 切换信息 */
	volatile sint target_change_num;	/* 渲染目标切换 */
}PiRenderInfo;

/* 全局变量，渲染信息实例指针 */
static PiRenderInfo *s_render_info = NULL;

PiRenderInfo* PI_API pi_renderinfo_get(void)
{
	if(s_render_info == NULL)
	{
		s_render_info = pi_new0(PiRenderInfo, 1);
	}
	return s_render_info;
}

void PI_API pi_renderinfo_set_current_renderer_name(const char *name) 
{
	PiRenderInfo *info = pi_renderinfo_get();
	info->curr_renderer_name = name;
}

const char* PI_API pi_renderinfo_get_current_renderer_name() 
{
	PiRenderInfo *info = pi_renderinfo_get();
	const char *name = info->curr_renderer_name;
	if(name == NULL)
		name = "";
	return name;
}

uint PI_API pi_renderinfo_get_texture_num(void)
{
	PiRenderInfo *info = pi_renderinfo_get();
	return pi_atomic_get(&info->tex_num);
}

uint PI_API pi_renderinfo_get_target_num(void)
{
	PiRenderInfo *info = pi_renderinfo_get();
	return pi_atomic_get(&info->target_num);
}

uint PI_API pi_renderinfo_get_vb_num(void)
{
	PiRenderInfo *info = pi_renderinfo_get();
	return pi_atomic_get(&info->vb_num);
}

uint PI_API pi_renderinfo_get_ib_num(void)
{
	PiRenderInfo *info = pi_renderinfo_get();
	return pi_atomic_get(&info->ib_num);
}

uint PI_API pi_renderinfo_get_view_num(void)
{
	PiRenderInfo *info = pi_renderinfo_get();
	return pi_atomic_get(&info->view_num);
}

uint PI_API pi_renderinfo_get_vs_num(void)
{
	PiRenderInfo *info = pi_renderinfo_get();
	return pi_atomic_get(&info->vs_num);
}

uint PI_API pi_renderinfo_get_fs_num(void)
{
	PiRenderInfo *info = pi_renderinfo_get();
	return pi_atomic_get(&info->fs_num);
}

uint PI_API pi_renderinfo_get_gpuprogram_num(void)
{
	PiRenderInfo *info = pi_renderinfo_get();
	return pi_atomic_get(&info->gpu_program_num);
}

uint PI_API pi_renderinfo_get_face_num(void)
{
	PiRenderInfo *info = pi_renderinfo_get();
	return pi_atomic_get(&info->face_num);
}

uint PI_API pi_renderinfo_get_vertex_num(void)
{
	PiRenderInfo *info = pi_renderinfo_get();
	return pi_atomic_get(&info->vertex_num);
}

uint PI_API pi_renderinfo_get_particle_num(void)
{
	PiRenderInfo *info = pi_renderinfo_get();
	return pi_atomic_get(&info->particle_num);
}

uint PI_API pi_renderinfo_get_entity_num(void)
{
	PiRenderInfo *info = pi_renderinfo_get();
	return pi_atomic_get(&info->entity_num);
}

uint PI_API pi_renderinfo_get_vb_change_num(void)
{
	PiRenderInfo *info = pi_renderinfo_get();
	return pi_atomic_get(&info->vb_change_num);
}

uint PI_API pi_renderinfo_get_ib_change_num(void)
{
	PiRenderInfo *info = pi_renderinfo_get();
	return pi_atomic_get(&info->ib_change_num);
}

uint PI_API pi_renderinfo_get_texture_change_num(void)
{
	PiRenderInfo *info = pi_renderinfo_get();
	return pi_atomic_get(&info->tex_change_num);
}

uint PI_API pi_renderinfo_get_gpuprogram_change_num(void)
{
	PiRenderInfo *info = pi_renderinfo_get();
	return pi_atomic_get(&info->gpuprogram_change_num);
}

uint PI_API pi_renderinfo_get_target_change_num(void)
{
	PiRenderInfo *info = pi_renderinfo_get();
	return pi_atomic_get(&info->target_change_num);
}

void PI_API pi_renderinfo_add_texture_num(sint num)
{
	PiRenderInfo *info = pi_renderinfo_get();
	pi_atomic_add(&info->tex_num, num);
}

void PI_API pi_renderinfo_add_target_num(sint num)
{
	PiRenderInfo *info = pi_renderinfo_get();
	pi_atomic_add(&info->target_num, num);
}

void PI_API pi_renderinfo_add_vb_num(sint num)
{
	PiRenderInfo *info = pi_renderinfo_get();
	pi_atomic_add(&info->vb_num, num);
}

void PI_API pi_renderinfo_add_ib_num(sint num)
{
	PiRenderInfo *info = pi_renderinfo_get();
	pi_atomic_add(&info->ib_num, num);
}

void PI_API pi_renderinfo_add_vs_num(sint num)
{
	PiRenderInfo *info = pi_renderinfo_get();
	pi_atomic_add(&info->vs_num, num);
}

void PI_API pi_renderinfo_add_fs_num(sint num)
{
	PiRenderInfo *info = pi_renderinfo_get();
	pi_atomic_add(&info->fs_num, num);
}

void PI_API pi_renderinfo_add_gpuprogram_num(sint num)
{
	PiRenderInfo *info = pi_renderinfo_get();
	pi_atomic_add(&info->gpu_program_num, num);
}

void PI_API pi_renderinfo_add_view_num(sint num)
{
	PiRenderInfo *info = pi_renderinfo_get();
	pi_atomic_add(&info->view_num, num);
}

void PI_API pi_renderinfo_add_face_num(sint num)
{
	PiRenderInfo *info = pi_renderinfo_get();
	pi_atomic_add(&info->face_num, num);
}

void PI_API pi_renderinfo_add_vertex_num(sint num)
{
	PiRenderInfo *info = pi_renderinfo_get();
	pi_atomic_add(&info->vertex_num, num);
}

void PI_API pi_renderinfo_add_particle_num(sint num)
{
	PiRenderInfo *info = pi_renderinfo_get();
	pi_atomic_add(&info->particle_num, num);
}

void PI_API pi_renderinfo_add_entity_num(sint num)
{
	PiRenderInfo *info = pi_renderinfo_get();
	pi_atomic_add(&info->entity_num, num);
}

void PI_API pi_renderinfo_add_vb_change_num(sint num)
{
	PiRenderInfo *info = pi_renderinfo_get();
	pi_atomic_add(&info->vb_change_num, num);
}

void PI_API pi_renderinfo_add_ib_change_num(sint num)
{
	PiRenderInfo *info = pi_renderinfo_get();
	pi_atomic_add(&info->ib_change_num, num);
}

void PI_API pi_renderinfo_add_texture_change_num(sint num)
{
	PiRenderInfo *info = pi_renderinfo_get();
	pi_atomic_add(&info->tex_change_num, num);
}

void PI_API pi_renderinfo_add_gpuprogram_change_num(sint num)
{
	PiRenderInfo *info = pi_renderinfo_get();
	pi_atomic_add(&info->gpuprogram_change_num, num);
}

void PI_API pi_renderinfo_add_target_change_num(sint num)
{
	PiRenderInfo *info = pi_renderinfo_get();
	pi_atomic_add(&info->target_change_num, num);
}

void PI_API pi_renderinfo_reset(void)
{
	PiRenderInfo *info = pi_renderinfo_get();
	pi_atomic_set(&info->face_num, 0);
	pi_atomic_set(&info->vertex_num, 0);
	pi_atomic_set(&info->entity_num, 0);
	pi_atomic_set(&info->ib_change_num, 0);
	pi_atomic_set(&info->vb_change_num, 0);
	pi_atomic_set(&info->tex_change_num, 0);
	pi_atomic_set(&info->target_change_num, 0);
	pi_atomic_set(&info->gpuprogram_change_num, 0);
	pi_atomic_set(&info->particle_num, 0);
}