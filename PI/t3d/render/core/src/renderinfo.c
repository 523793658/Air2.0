#include <renderinfo.h>

typedef struct  
{
	/* ��ǰ���е���Ⱦ�������� */
	const char *curr_renderer_name;

	/* ��Ⱦ��Դ��� */
	volatile sint tex_num;		/* �������� */
	volatile sint ib_num;		/* ���������� */
	volatile sint vb_num;		/* ������������ */
	volatile sint target_num;	/* RenderTarget������ */
	volatile sint view_num;		/* RenderView������ */
	volatile sint vs_num;		/* vs������ */
	volatile sint fs_num;		/* fs������ */
	volatile sint gpu_program_num;	/* GPU program ������ */

	/* ÿ֡��Ⱦ��� */
	volatile sint face_num;		/* ��֡��Ⱦ������ */
	volatile sint vertex_num;	/* ��֡��Ⱦ�Ķ����� */
	volatile sint entity_num;	/* ��֡��Ⱦ��DrawCall�� */
	volatile sint particle_num; /* ��֡��Ⱦ��������
	
	/* ÿ֡�л���Ϣ */
	volatile sint vb_change_num;	/* VB�л���Ϣ */
	volatile sint ib_change_num;	/* IB�л���Ϣ */
	volatile sint tex_change_num;	/* TEX�л���Ϣ */
	volatile sint gpuprogram_change_num;	/* �л���Ϣ */
	volatile sint target_change_num;	/* ��ȾĿ���л� */
}PiRenderInfo;

/* ȫ�ֱ�������Ⱦ��Ϣʵ��ָ�� */
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