#include <vertex_anim.h>

typedef struct
{
	uint32 node_count;				//entity的节点数
	uint curr_entity_index;			//控制器当前apply的节点索引
	PiBool is_stop;

	float speed;
	float start_time;
	float curr_time;

	VertexAnimationState state;
	PiVertexAnim *curr_anim;
	PiMesh **meshes;				//当前网格的指针数组
	PiRenderMesh **rmeshes;			//当前rmesh的指针数组

	Transform ** *uv_transforms;	//当前uv变换指针的二维数组

	int32 play_time;
	PiHash *anim_map;

	/* 字符串常量 */
	char *defs[TexMapTypeNum];
	char *uniforms[TexMapTypeNum];
} VertexAnimate;

static PiBool _apply(PiController *c, ControllerApplyType type, void *obj)
{
	PiEntity *entity;
	VertexAnimate *impl = (VertexAnimate *)c->impl;
	uint32 sub_map_index;
	PiMatrix4 transform_mat;
	float transform_mat3[3][3];
	PI_ASSERT(type == CAT_ENTITY, "apply vertex_anim controller failed, accept entity type, type = %d", type);
	entity = (PiEntity *)obj;

	if (impl->curr_anim != NULL && impl->meshes != NULL)
	{
		if (entity->mesh != NULL)
		{
			pi_free(entity->mesh);
			entity->mesh = NULL;
		}
		entity->mesh = pi_rendermesh_new(impl->meshes[impl->curr_entity_index], TRUE);

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

void PI_API _modify_meshes(VertexAnimate *impl, int32 time)
{
	int32 j, pre = 0, next = 0, sub_map_index;
	uint32 i;
	float weight;

	if (impl->curr_anim != NULL)
	{
		if (time >= impl->play_time)
		{
			time %= impl->play_time;
		}

		for (i = 0; i < impl->curr_anim->node_count; ++i)
		{
			for (j = 0; j < impl->curr_anim->nodes[i].frame_count; ++j)
			{
				if (time > impl->curr_anim->nodes[i].frames[j].start_time)
				{
					pre = j;
				}
				else if (time == impl->curr_anim->nodes[i].frames[j].start_time)
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

			if (impl->meshes[i] == NULL)
			{
				impl->meshes[i] = pi_mesh_create_empty();
			}

			pi_mesh_copy(impl->meshes[i], impl->curr_anim->nodes[i].frames[pre].meshes[0]);

			if (impl->uv_transforms[i] == NULL)
			{
				impl->uv_transforms[i] = pi_new0(Transform *, TexMapTypeNum);
			}

			for (sub_map_index = 0; sub_map_index < TexMapTypeNum; ++sub_map_index)
			{

				if (impl->curr_anim->nodes[i].frames[pre].uv_transforms[sub_map_index] != NULL)
				{
					if (impl->uv_transforms[i][sub_map_index] == NULL)
					{
						impl->uv_transforms[i][sub_map_index] = pi_new0(Transform, 1);
					}

					pi_vec3_copy(&impl->uv_transforms[i][sub_map_index]->translate, &impl->curr_anim->nodes[i].frames[pre].uv_transforms[sub_map_index]->translate);
					pi_vec3_copy(&impl->uv_transforms[i][sub_map_index]->scale, &impl->curr_anim->nodes[i].frames[pre].uv_transforms[sub_map_index]->scale);
					pi_quat_copy(&impl->uv_transforms[i][sub_map_index]->rotate, &impl->curr_anim->nodes[i].frames[pre].uv_transforms[sub_map_index]->rotate);
				}
			}

			//当前时间在两帧之间 对网格和变换矩阵进行插值
			if (pre != next)
			{
				weight = (float)(time - impl->curr_anim->nodes[i].frames[pre].start_time) / (float)(impl->curr_anim->nodes[i].frames[next].start_time - impl->curr_anim->nodes[i].frames[pre].start_time);
				pi_mesh_interpolate(impl->meshes[i], impl->curr_anim->nodes[i].frames[pre].meshes[0], impl->curr_anim->nodes[i].frames[next].meshes[0], weight);

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

static PiBool _update(PiController *c, float tpf)
{

	VertexAnimate *impl = (VertexAnimate *)c->impl;
	float real_time;

	if (impl->is_stop)
	{
		return TRUE;
	}

	impl->curr_time += tpf * 1000.0f * impl->speed;
	real_time = impl->curr_time - impl->start_time;

	if (real_time >= impl->play_time && impl->state != VAS_LOOP)
	{
		real_time = (float)impl->play_time;
	}

	_modify_meshes(impl, (int32)real_time);

	if (real_time >= impl->play_time)
	{
		switch (impl->state)
		{
			case VAS_LOOP:
				impl->is_stop = FALSE;
				break;

			case VAS_KEEP:
				impl->is_stop = FALSE;
				impl->curr_time = 0.0f;//(float)impl->curr_anim->nodes[0]->frames[0]->start_time;
				impl->start_time = 0.0f;

				impl->curr_anim = NULL;
				break;

			default:

				pi_vertex_anim_stop(c);
				break;
		}
	}

	return FALSE;
}

PiController *PI_API pi_vertex_anim_new(uint32 node_count)
{
	VertexAnimate *impl = pi_new0(VertexAnimate, 1);
	PiController *c = pi_controller_new(CT_VERTEX_ANIM, _apply, _update, impl);

	impl->is_stop = TRUE;
	impl->anim_map = pi_hash_new(0.75f, pi_wstr_hash, pi_wstring_equal);
	impl->meshes = NULL;
	impl->rmeshes = NULL;
	impl->node_count = node_count;
	impl->curr_anim = NULL;
	impl->meshes = pi_new0(PiMesh *, node_count);
	impl->rmeshes = pi_new0(PiRenderMesh *, node_count);

	impl->uv_transforms = pi_new0(Transform **, node_count);

	impl->defs[DiffuseColor] = pi_conststr("DIFFUSE_MAP_MATRIX");
	impl->defs[SelfIllumination] = pi_conststr("GLOW_MAP_MATRIX");
	impl->defs[Opacity] = pi_conststr("ALPHA_MAP_MATRIX");

	impl->uniforms[DiffuseColor] = pi_conststr("u_DiffuseTexMatrix");
	impl->uniforms[SelfIllumination] = pi_conststr("u_GlowTexMatrix");
	impl->uniforms[Opacity] = pi_conststr("u_AlphaTexMatrix");

	return c;
}

static PiSelectR PI_API _delete_key(void *user_data, PiKeyValue  *value)
{
	pi_free(value->key);
	value->key = NULL;
	return SELECT_NEXT;
}

void PI_API pi_vertex_anim_free(PiController *c)
{
	VertexAnimate *impl = (VertexAnimate *)c->impl;
	uint32 i;
	int32 sub_map_index;

	if (impl == NULL)
	{
		return;
	}

	for (i = 0; i < impl->node_count; ++i)
	{
		if (impl->meshes[i] != NULL)
		{
			pi_mesh_free(impl->meshes[i]);
		}

		impl->meshes[i] = NULL;

		if (impl->rmeshes[i] != NULL)
		{
			pi_rendermesh_free(impl->rmeshes[i]);
		}

		impl->rmeshes[i] = NULL;

		if (impl->uv_transforms[i] != NULL)
		{
			for (sub_map_index = 0; sub_map_index < TexMapTypeNum; ++sub_map_index)
			{
				if (impl->uv_transforms[i] != NULL && impl->uv_transforms[i][sub_map_index] != NULL)
				{
					pi_free(impl->uv_transforms[i][sub_map_index]);
					impl->uv_transforms[i][sub_map_index] = NULL;
				}
			}

			if (impl->uv_transforms[i] != NULL && impl->uv_transforms[i] != NULL)
			{
				pi_free(impl->uv_transforms[i]);
				impl->uv_transforms[i] = NULL;
			}
		}
	}

	if (impl->meshes != NULL)
	{
		pi_free(impl->meshes);
		impl->meshes = NULL;
	}

	if (impl->rmeshes != NULL)
	{
		pi_free(impl->rmeshes);
		impl->rmeshes = NULL;
	}

	if (impl->uv_transforms != NULL)
	{
		pi_free(impl->uv_transforms);
		impl->uv_transforms = NULL;
	}

	if (impl->anim_map != NULL)
	{
		pi_hash_clear(impl->anim_map, FALSE);
		pi_free(impl->anim_map);
		impl->anim_map = NULL;
	}

	pi_free(impl);
	impl = NULL;
	/*pi_controller_free(c);
	c = NULL;*/
}

PiBool PI_API pi_vertex_anim_add(PiController *c, wchar *name, PiVertexAnim *anim)
{
	PiKeyValue old;
	VertexAnimate *impl = (VertexAnimate *)c->impl;
	wchar *anim_name = pi_wstr_dup(name);

	if (pi_hash_enter(impl->anim_map, anim_name, anim, &old))
	{
		pi_free(&old);
	}

	PI_ASSERT(impl->node_count == anim->node_count, "Entity Num = %d but Animation nodes count = %d", impl->node_count, anim->node_count);

	return TRUE;
}

PiBool PI_API pi_vertex_anim_play(PiController *c, wchar *name, float speed, VertexAnimationState state, float start_time)
{
	VertexAnimate *impl = (VertexAnimate *)c->impl;
	PiVertexAnim *anim = NULL;
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

PiBool PI_API pi_vertex_anim_stop(PiController *c)
{
	VertexAnimate *impl = (VertexAnimate *)c->impl;

	if (impl->is_stop)
	{
		return TRUE;
	}

	switch (impl->state)
	{
		case VAS_KEEP:

			break;

		case VAS_FIRST:
			_modify_meshes(impl, 0);
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

PiVertexAnim *PI_API pi_veanim_new(byte *data, uint32 size)
{
	PiVertexAnim *anim = pi_new0(PiVertexAnim, 1);
	pi_veanim_load(anim, data, size);
	return anim;
}

void PI_API pi_veanim_load(PiVertexAnim *veanim, byte *data, uint32 size)
{
	uint32 mesh_num, i;
	PiBytes mesh_num_buffer, duration_buffer;

	pi_memset_inline(veanim, 0, sizeof(veanim));
	//读取节点数
	pi_bytes_load(&mesh_num_buffer, data, sizeof(uint32), FALSE);
	pi_bytes_read_uint32(&mesh_num_buffer, &mesh_num);
	data += sizeof(uint32);
	pi_bytes_clear(&mesh_num_buffer, FALSE);
	veanim->nodes = pi_new0(VertexAnimNode, mesh_num);
	veanim->node_count = mesh_num;

	//遍历节点
	for (i = 0; i < mesh_num; ++i)
	{
		int32 j, frame_count;
		PiBytes frame_count_buffer;

		pi_bytes_load(&frame_count_buffer, data, sizeof(int32), FALSE);
		pi_bytes_read_int32(&frame_count_buffer, &frame_count);
		data += sizeof(int32);
		pi_bytes_clear(&frame_count_buffer, FALSE);
		veanim->nodes[i].frame_count = frame_count;
		veanim->nodes[i].frames = pi_new0(VertexAnimKeyFrame, frame_count);

		for (j = 0; j < frame_count; ++j)
		{
			uint32 model_size, sub_mesh_num, k;
			int32 start_time;
			PiBytes key_time_buffer;
			PiBytes model_size_buffer;
			VertexAnimKeyFrame *frame = NULL;
			frame = veanim->nodes[i].frames + j;

			frame->uv_transforms = pi_new0(Transform *, TexMapTypeNum);
			//读取关键帧时间
			pi_bytes_load(&key_time_buffer, data, sizeof(int32), FALSE);
			pi_bytes_read_int32(&key_time_buffer, &start_time);
			frame->start_time = start_time;
			pi_bytes_clear(&key_time_buffer, FALSE);
			data += sizeof(int32);

			//读取关键帧模型大小
			pi_bytes_load(&model_size_buffer, data, sizeof(uint32), FALSE);
			pi_bytes_read_uint32(&model_size_buffer, &model_size);
			pi_bytes_clear(&model_size_buffer, FALSE);
			data += sizeof(uint32);

			//模型文件
			sub_mesh_num = pi_mesh_num(data, model_size);
			frame->meshes = pi_new0(PiMesh *, sub_mesh_num);
			frame->sub_mesh_count = sub_mesh_num;

			for (k = 0; k < sub_mesh_num; ++k)
			{
				frame->meshes[k] = pi_new0(PiMesh, 1);
			}

			pi_mesh_load(frame->meshes, sub_mesh_num, data, model_size);

			data += model_size;

			//材质偏移
			for (k = 0; k < sub_mesh_num; ++k)
			{
				int32 has_mtl = 0;
				uint32 sub_map_index, num_sub_texmaps;
				PiBytes has_mtl_buffer, float_buffer, num_sub_texmaps_buffer;
				pi_bytes_load(&has_mtl_buffer, data, sizeof(int32), FALSE);
				pi_bytes_read_int32(&has_mtl_buffer, &has_mtl);
				data += sizeof(int32);

				if (has_mtl > 0)
				{
					pi_bytes_load(&num_sub_texmaps_buffer, data, sizeof(uint32), FALSE);
					pi_bytes_read_uint32(&num_sub_texmaps_buffer, &num_sub_texmaps);
					data += sizeof(uint32);

					for (sub_map_index = 0; sub_map_index < num_sub_texmaps; ++sub_map_index)
					{
						PiBytes has_tex_buffer;
						int32 has_tex;

						pi_bytes_load(&has_tex_buffer, data, sizeof(int32), FALSE);
						pi_bytes_read_int32(&has_tex_buffer, &has_tex);
						data += sizeof(int32);

						if (has_tex > 0)
						{
							if (frame->uv_transforms[sub_map_index] == NULL)
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

	pi_bytes_load(&duration_buffer, data, sizeof(uint), FALSE);
	pi_bytes_read_int32(&duration_buffer, &veanim->duration);
	pi_bytes_clear(&duration_buffer, FALSE);
}

void PI_API pi_veanim_free(PiVertexAnim *anim)
{
	uint32 i, k;
	int32 j;

	if (anim != NULL)
	{
		if (anim->nodes != NULL)
		{
			for (i = 0; i < anim->node_count; ++i)
			{
				if (anim->nodes[i].frames != NULL)
				{
					for (j = 0; j < anim->nodes[i].frame_count; ++j)
					{
						if (anim->nodes[i].frames[j].meshes != NULL)
						{
							for (k = 0; k < anim->nodes[i].frames[j].sub_mesh_count; ++k)
							{
								if (anim->nodes[i].frames[j].meshes[k] != NULL)
								{
									pi_mesh_free(anim->nodes[i].frames[j].meshes[k]);
									anim->nodes[i].frames[j].meshes[k] = NULL;
								}
							}

							pi_free(anim->nodes[i].frames[j].meshes);
							anim->nodes[i].frames[j].meshes = NULL;
						}

						if (anim->nodes[i].frames[j].uv_transforms != NULL)
						{
							for (k = 0; k < TexMapTypeNum; ++k)
							{
								if (anim->nodes[i].frames[j].uv_transforms[k] != NULL)
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
