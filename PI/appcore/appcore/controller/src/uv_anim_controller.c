#include "uv_anim_controller.h"

typedef struct
{
	uint32 node_count;				//entity的节点数
	uint curr_entity_index;			//控制器当前apply的节点索引
	PiBool is_stop;

	float speed;
	float start_time;
	float curr_time;

	UVAnimationState state;
	PiUVAnim *curr_anim;
	Transform ** *uv_transforms;	//当前uv变换指针的二维数组

	int32 play_time;
	PiHash *anim_map;

	/* 字符串常量 */
	char *defs[TexMapTypeNum];
	char *uniforms[TexMapTypeNum];
} UVAnimate;

void PI_API _modify_uv(UVAnimate *impl, int32 time)
{
	int32 j, pre = 0, next = 0, sub_map_index;
	uint32 i;
	float weight;
	if(impl->curr_anim != NULL)
	{
		if(time >= impl->play_time)
		{
			time %= impl->play_time;
		}
		for(i = 0 ; i < impl->curr_anim->node_count; ++i)
		{
			for (j = 0; j < impl->curr_anim->nodes[i].frame_count; j++)
			{
				if(time > impl->curr_anim->nodes[i].frames[j].start_time)
				{
					pre = j;
				}
				else if(time == impl->curr_anim->nodes[i].frames[j].start_time)
				{
					pre = next = j;
					break;
				}
				else
				{
					next = j;
					break;
				}
			}
			if(impl->uv_transforms[i] == NULL)
			{
				impl->uv_transforms[i] = pi_new0(Transform*, TexMapTypeNum);
			}
			for (sub_map_index = 0 ; sub_map_index < TexMapTypeNum; ++sub_map_index)
			{
				if(impl->curr_anim->nodes[i].frames[pre].uv_transforms[sub_map_index] != NULL)
				{
					if(impl->uv_transforms[i][sub_map_index] == NULL)
					{
						impl->uv_transforms[i][sub_map_index] = pi_new0(Transform, 1);
					}
					pi_vec3_copy(&impl->uv_transforms[i][sub_map_index]->translate, &impl->curr_anim->nodes[i].frames[pre].uv_transforms[sub_map_index]->translate);
					pi_vec3_copy(&impl->uv_transforms[i][sub_map_index]->scale, &impl->curr_anim->nodes[i].frames[pre].uv_transforms[sub_map_index]->scale);
					pi_quat_copy(&impl->uv_transforms[i][sub_map_index]->rotate, &impl->curr_anim->nodes[i].frames[pre].uv_transforms[sub_map_index]->rotate);
				}
			}
			if(pre != next)
			{
				weight = (float)(time - impl->curr_anim->nodes[i].frames[pre].start_time) / (float)(impl->curr_anim->nodes[i].frames[next].start_time - impl->curr_anim->nodes[i].frames[pre].start_time);
				for (sub_map_index = 0; sub_map_index < TexMapTypeNum; ++sub_map_index)
				{
					if (impl->uv_transforms[i][sub_map_index] != NULL)
					{
						pi_vec3_lerp(&impl->uv_transforms[i][sub_map_index]->translate, &impl->curr_anim->nodes[i].frames[pre].uv_transforms[sub_map_index]->translate, &impl->curr_anim->nodes[i].frames[next].uv_transforms[sub_map_index]->translate, weight);
						pi_vec3_lerp(&impl->uv_transforms[i][sub_map_index]->scale, &impl->curr_anim->nodes[i].frames[pre].uv_transforms[sub_map_index]->scale, &impl->curr_anim->nodes[i].frames[next].uv_transforms[sub_map_index]->scale, weight);
						pi_quat_lerp(&impl->uv_transforms[i][sub_map_index]->rotate, &impl->curr_anim->nodes[i].frames[pre].uv_transforms[sub_map_index]->rotate, &impl->curr_anim->nodes[i].frames[next].uv_transforms[sub_map_index]->rotate, weight, FALSE);
					}
				}
			}
		}
	}
	impl->curr_entity_index = 0;
}

PiBool PI_API app_uv_anim_stop(PiController *c)
{
	UVAnimate *impl = (UVAnimate*) c->impl;
	if(impl->is_stop)
	{
		return TRUE;
	}
	switch (impl->state)
	{
	case UV_KEEP:
		break;
	case UV_FIRST:
		_modify_uv(impl, 0);
		break;
	default:
		break;
	}
	impl->curr_time = 0.0f;
	impl->start_time = 0.0f;
	impl->curr_anim = NULL;
	impl->is_stop = TRUE;
	return TRUE;
}

PiBool PI_API app_uv_anim_play(PiController *c, wchar *name, float speed, UVAnimationState state, float start_time)
{
	UVAnimate *impl = (UVAnimate *)c->impl;
	PiUVAnim *anim = NULL;
	PiBool result = pi_hash_lookup(impl->anim_map, name, (void **)&anim);

	if (result)
	{
		impl->is_stop = FALSE;
		impl->curr_time = start_time * 1000.0f + anim->nodes[0].frames[0].start_time;
		impl->start_time = 0.0f;
		impl->state = state;
		impl->speed = speed;
		impl->curr_anim = anim;

		impl->play_time = anim->duration;
	}

	return result;
}


static PiBool _apply(PiController *c, ControllerApplyType type, void *obj)
{
	PiEntity *entity;
	UVAnimate *impl = (UVAnimate *)c->impl;
	uint32 sub_map_index;
	PiMatrix4 transform_mat;
	float transform_mat3[3][3];
	PI_ASSERT(type == CAT_ENTITY, "apply vertex_anim controller failed, accept entity type, type = %d", type);
	entity = (PiEntity *)obj;

	if (impl->curr_anim != NULL)
	{

		if (impl->uv_transforms[impl->curr_entity_index] != NULL)
		{
			for (sub_map_index = 0; sub_map_index < TexMapTypeNum; ++sub_map_index)
			{
				if (impl->uv_transforms[impl->curr_entity_index][sub_map_index] != NULL)
				{
					pi_mat4_set_identity(&transform_mat);
					pi_mat4_build_transform(&transform_mat,
						&impl->uv_transforms[impl->curr_entity_index][sub_map_index]->translate,
						&impl->uv_transforms[impl->curr_entity_index][sub_map_index]->scale,
						&impl->uv_transforms[impl->curr_entity_index][sub_map_index]->rotate);
					pi_material_set_def(entity->material, impl->defs[sub_map_index], TRUE);

					transform_mat3[0][0]= transform_mat.m[0][0];
					transform_mat3[0][1]= transform_mat.m[0][1];
					transform_mat3[0][2]= transform_mat.m[0][3];
					transform_mat3[1][0]= transform_mat.m[1][0];
					transform_mat3[1][1]= transform_mat.m[1][1];
					transform_mat3[1][2]= transform_mat.m[1][3];
					transform_mat3[2][0]= transform_mat.m[3][0];
					transform_mat3[2][1]= transform_mat.m[3][1];
					transform_mat3[2][2]= transform_mat.m[3][3];

					pi_material_set_uniform(entity->material, impl->uniforms[sub_map_index], UT_MATRIX3, 1, transform_mat3, TRUE);
				}
			}

		}

		++impl->curr_entity_index;
	}

	return TRUE;
}
static PiBool _update(PiController *c, float tpf)
{
	UVAnimate *impl = (UVAnimate*) c->impl;
	float real_time;
	if(impl->is_stop)
	{
		return TRUE;
	}
	impl->curr_time += tpf * 1000.0f * impl->speed;
	real_time = impl->curr_time - impl->start_time;

	if(real_time >=impl->play_time && impl->state != UV_LOOP)
	{
		real_time = (float)impl->play_time;
	}
	_modify_uv(impl, (int32)real_time);

	if(real_time >= impl->play_time)
	{
		switch (impl->state)
		{
		case UV_LOOP:
			impl->is_stop = FALSE;
			break;
		case UV_KEEP:
			impl->is_stop = FALSE;
			impl->curr_time = 0.0f;
			impl->start_time = 0.0f;
			impl->curr_anim = NULL;
			break;
		default:
			app_uv_anim_stop(c);
			break;
		}
	}
	return TRUE;

}

PiBool PI_API app_uv_anim_add(PiController *c, wchar *name, PiUVAnim *anim)
{
	PiKeyValue old;
	UVAnimate *impl = (UVAnimate*)c->impl;
	wchar *anim_name = pi_wstr_dup(name);
	if(pi_hash_enter(impl->anim_map, anim_name, anim, &old))
	{
		pi_free(&old);
	}
	PI_ASSERT(impl->node_count == anim->node_count, "Entity Num = %d but Animation nodes count = %d", impl->node_count, anim->node_count);

	return TRUE;
}

PiController* PI_API app_uv_anim_controller_new(uint32 node_count)
{
	UVAnimate *impl = pi_new0(UVAnimate, 1);
	PiController *c = pi_controller_new((ControllerType)CT_UVANIM, _apply, _update, impl);

	impl->is_stop = TRUE;
	impl->anim_map = pi_hash_new(0.75f, pi_wstr_hash, pi_wstring_equal);
	impl->node_count = node_count;
	impl->curr_anim = NULL;
	impl->uv_transforms = pi_new0(Transform**, node_count);

	impl->defs[DiffuseColor] = pi_conststr("DIFFUSE_MAP_MATRIX");
	impl->defs[SelfIllumination] = pi_conststr("GLOW_MAP_MATRIX");
	impl->defs[Opacity] = pi_conststr("ALPHA_MAP_MATRIX");

	impl->uniforms[DiffuseColor] = pi_conststr("u_DiffuseTexMatrix");
	impl->uniforms[SelfIllumination] = pi_conststr("u_GlowTexMatrix");
	impl->uniforms[Opacity] = pi_conststr("u_AlphaTexMatrix");
	return c;
}

void PI_API app_uv_anim_controller_free(PiController* controller)
{
	UVAnimate *impl = (UVAnimate*)controller->impl;
	uint32 i, sub_map_index;
	if(impl != NULL)
	{
		for(i = 0 ; i < impl->node_count; i++)
		{
			if(impl->uv_transforms[i] != NULL)
			{
				for(sub_map_index = 0; sub_map_index < TexMapTypeNum; sub_map_index++)
				{
					if(impl->uv_transforms[i] != NULL && impl->uv_transforms[i][sub_map_index] != NULL)
					{
						pi_free(impl->uv_transforms[i][sub_map_index]);
						impl->uv_transforms[i][sub_map_index] = NULL;
					}
				}
				if(impl->uv_transforms[i] != NULL)
				{
					pi_free(impl->uv_transforms[i]);
					impl->uv_transforms[i] = NULL;
				}
			}
		}
		if(impl->uv_transforms != NULL)
		{
			pi_free(impl->uv_transforms);
			impl->uv_transforms = NULL;
		}
		if(impl->anim_map != NULL)
		{
			pi_hash_clear(impl->anim_map, FALSE);
			pi_free(impl->anim_map);
			impl->anim_map = NULL;
		}
		pi_free(impl);
		impl = NULL;
	}
	pi_controller_free(controller);
}

void PI_API app_uv_anim_free(PiUVAnim *anim)
{
	uint32 i, k;
	int32 j;
	if(anim != NULL)
	{
		if(anim->nodes != NULL)
		{
			for(i = 0 ; i < anim->node_count; i++)
			{
				if(anim->nodes[i].frames != NULL)
				{
					for(j = 0; j < anim->nodes[i].frame_count; j++)
					{
						if(anim->nodes[i].frames[j].uv_transforms != NULL)
						{
							for(k = 0 ; k < TexMapTypeNum; k++)
							{
								if(anim->nodes[i].frames[j].uv_transforms[k] != NULL)
								{
									pi_free(anim->nodes[i].frames[j].uv_transforms[k]);
									anim->nodes[i].frames[j].uv_transforms[k] = NULL;
								}
							}
							pi_free(anim->nodes[i].frames[j].uv_transforms);
							anim->nodes[i].frames[j].uv_transforms = NULL;
						}
					}
					pi_free(anim->nodes[i].frames);
					anim->nodes[i].frames = NULL;
				}
			}
			pi_free(anim->nodes);
			anim->nodes = NULL;
		}
		pi_free(anim);
	}
}

void PI_API app_uvanim_load(PiUVAnim *uvanim, byte* data, uint32 size)
{
	uint32 mesh_num, i;
	PiBytes mesh_num_buffer, duration_buffer;
	pi_memset_inline(uvanim, 0, sizeof(uvanim));

	pi_bytes_load(&mesh_num_buffer, data, sizeof(uint32), FALSE);
	pi_bytes_read_uint32(&mesh_num_buffer, &mesh_num);

	data += sizeof(uint32);
	pi_bytes_clear(&mesh_num_buffer, FALSE);
	uvanim->nodes = pi_new0(UVAnimNode, mesh_num);
	uvanim->node_count = mesh_num;

	for(i = 0 ; i < mesh_num; i++)
	{
		int32 j, frame_count;
		PiBytes frame_count_buffer;

		pi_bytes_load(&frame_count_buffer, data, sizeof(int32), FALSE);
		pi_bytes_read_int32(&frame_count_buffer, &frame_count);
		data += sizeof(int32);
		pi_bytes_clear(&frame_count_buffer, FALSE);
		uvanim->nodes[i].frame_count = frame_count;
		uvanim->nodes[i].frames = pi_new0(UVAnimKeyFrame, frame_count);
		for(j = 0 ; j < frame_count; j++)
		{
			PiBytes key_time_buffer, model_size_buffer, sub_mesh_count_buffer;
			int32 start_time;
			uint32 model_size, sub_mesh_count, k;
			UVAnimKeyFrame* frame = NULL;
			frame = uvanim->nodes[i].frames + j;
			frame->uv_transforms = pi_new0(Transform*, TexMapTypeNum);

			pi_bytes_load(&key_time_buffer, data, sizeof(int32), FALSE);
			pi_bytes_read_int32(&key_time_buffer, &start_time);
			frame->start_time = start_time;
			pi_bytes_clear(&key_time_buffer, FALSE);
			data += sizeof(int32);

			pi_bytes_load(&model_size_buffer, data, sizeof(uint32), FALSE);
			pi_bytes_read_uint32(&model_size_buffer, &model_size);
			pi_bytes_clear(&model_size_buffer, FALSE);
			data += sizeof(uint32);

			pi_bytes_load(&sub_mesh_count_buffer, data, sizeof(uint32), FALSE);
			pi_bytes_read_uint32(&sub_mesh_count_buffer, &sub_mesh_count);
			frame->sub_mesh_count = sub_mesh_count;
			pi_bytes_clear(&sub_mesh_count_buffer, FALSE);
			data += sizeof(uint32);

			for(k = 0; k < sub_mesh_count; k++)
			{
				int32 has_mtl = 0;
				uint32 sub_map_index, num_sum_texmaps;
				PiBytes has_mtl_buffer, float_buffer, num_sum_texmaps_buffer;
				pi_bytes_load(&has_mtl_buffer, data, sizeof(int32), FALSE);
				pi_bytes_read_int32(&has_mtl_buffer, &has_mtl);
				data += sizeof(int32);
				if(has_mtl >0)
				{
					pi_bytes_load(&num_sum_texmaps_buffer, data, sizeof(uint32), FALSE);
					pi_bytes_read_uint32(&num_sum_texmaps_buffer, &num_sum_texmaps);
					data += sizeof(uint32);
					for(sub_map_index = 0; sub_map_index < num_sum_texmaps; sub_map_index++)
					{
						PiBytes has_tex_buffer;
						int32 has_tex;

						pi_bytes_load(&has_tex_buffer, data, sizeof(int32), FALSE);
						pi_bytes_read_int32(&has_tex_buffer, &has_tex);
						data += sizeof(int32);
						if(has_tex > 0)
						{
							if(frame->uv_transforms[sub_map_index] == NULL)
							{
								frame->uv_transforms[sub_map_index] = pi_new0(Transform, 1);
							}
							pi_bytes_load(&float_buffer, data, sizeof(float), FALSE);
							pi_bytes_read_float(&float_buffer, &frame->uv_transforms[sub_map_index]->translate.x);
							data += sizeof(float);
							pi_bytes_load(&float_buffer, data, sizeof(float), FALSE);
							pi_bytes_read_float(&float_buffer, &frame->uv_transforms[sub_map_index]->translate.y);
							data += sizeof(float);
							pi_bytes_load(&float_buffer, data, sizeof(float), FALSE);
							pi_bytes_read_float(&float_buffer, &frame->uv_transforms[sub_map_index]->translate.z);
							data += sizeof(float);

							pi_bytes_load(&float_buffer, data, sizeof(float), FALSE);
							pi_bytes_read_float(&float_buffer, &frame->uv_transforms[sub_map_index]->scale.x);
							data += sizeof(float);
							pi_bytes_load(&float_buffer, data, sizeof(float), FALSE);
							pi_bytes_read_float(&float_buffer, &frame->uv_transforms[sub_map_index]->scale.y);
							data += sizeof(float);
							pi_bytes_load(&float_buffer, data, sizeof(float), FALSE);
							pi_bytes_read_float(&float_buffer, &frame->uv_transforms[sub_map_index]->scale.z);
							data += sizeof(float);


							pi_bytes_load(&float_buffer, data, sizeof(float), FALSE);
							pi_bytes_read_float(&float_buffer, &frame->uv_transforms[sub_map_index]->rotate.x);
							data += sizeof(float);
							pi_bytes_load(&float_buffer, data, sizeof(float), FALSE);
							pi_bytes_read_float(&float_buffer, &frame->uv_transforms[sub_map_index]->rotate.y);
							data += sizeof(float);
							pi_bytes_load(&float_buffer, data, sizeof(float), FALSE);
							pi_bytes_read_float(&float_buffer, &frame->uv_transforms[sub_map_index]->rotate.z);
							data += sizeof(float);
							pi_bytes_load(&float_buffer, data, sizeof(float), FALSE);
							pi_bytes_read_float(&float_buffer, &frame->uv_transforms[sub_map_index]->rotate.w);
							data += sizeof(float);

							pi_bytes_clear(&float_buffer, FALSE);
						}
					}
				}
			}
		}
	}
	pi_bytes_load(&duration_buffer, data, sizeof(uint32), FALSE);
	pi_bytes_read_int32(&duration_buffer, &uvanim->duration);
	pi_bytes_clear(&duration_buffer, FALSE);
}

PiUVAnim* PI_API app_uv_anim_new(byte* data, uint32 size){
	PiUVAnim *anim = pi_new0(PiUVAnim, 1);
	app_uvanim_load(anim, data, size);
	return anim;
}


