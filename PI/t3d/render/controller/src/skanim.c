
#include <skanim.h>
#include <pi_skeleton.h>




typedef struct  
{
	char* name;
	float transient_time;
	float speed;
	AnimationState state;
	float start_time;
	char* defaultName;
} BoneAnimaInfo;

typedef struct
{
	PiBool is_stop;			/* 是否停止播放 */
	PiBool is_cache_anim;
	PiSkeleton *sk;			/* 骨骼指针 */
	Ragdoll* ragdoll;

	SkeletonAnimType type;

	float* cach_matrix_impl;

	float speed;			/* 速度比例 */
	float start_time;		/* 过渡的开始时间，单位：毫秒 */
	float curr_time;		/* 当前时间，单位：毫秒 */
	float transient_time;	/* 过渡时间，单位：毫秒 */

	PiSkeleton *last_anim;		/* 上一个播放的动画 */
	BoneTransform *last_bones;	/* 骨头数据 */

	AnimationState state;	/* 动画状态 */
	PiSkeleton *play_anim;	/* 正在播放的动画 */

	uint play_time;			/* 当前动画的播放时间，单位：毫秒 */

	uint num_bones;			 /* 骨头数量 */
	float *bone_mat_columns; /* 骨头矩阵数组对应的uniform变量，3个vec4值 */
	PiMatrix4 *bone_mats;	/* 骨头矩阵数组 */
	BoneTransform *bones;	/* 骨头数据 */

	int bone_info;

	PiHash anim_map;		/* key=wchar*, value=PiSkeleton* */

	PiQueue* animQueue;
	char* defaultName;

	/* 常量字符串 */
	char *HARDWARE_SKELETON;
	char *RAGDOLL;

	char *U_boneMatrices;
	char *U_boneInfo;
} SkeletonAnimate;

static PiBool interpolate = TRUE;

static BoneAnimaInfo *_create_anim_info();

static void _free_anim_info();

/* 创建骨头数据 */
static BoneTransform *_create_bones(PiSkeleton *sk);

/* 释放骨头数据 */
static void _destroy_bones(BoneTransform *bones);

/* 克隆骨头数据 */
static BoneTransform *_clone_bones(BoneTransform *bones);

/* 拷贝骨头数据 */
static void _copy_bones(BoneTransform *dst, BoneTransform *src);

/* 骨头数据设为初始姿态 */
static void _init_bones(PiSkeleton *sk, BoneTransform *bones);

/* 获取bone_id对应的初始骨头矩阵 */
static void _get_init_bone_matrix(PiSkeleton *sk, PiMatrix4 *dst, uint bone_id);

/**
 * 将动画作用于骨头
 * time: 动画时间，单位：毫秒
 */
static void _anim_bones(BoneTransform *bones, PiSkeleton *sk, uint time);

/* 骨头混合 */
static void _blend_bones(BoneTransform *dst, BoneTransform *src1, BoneTransform *src2, float weight);

/* 取到骨头的姿态 */
static void _bind_bones(BoneTransform *bones, PiSkeleton *sk);

/* 得到骨骼矩阵 */
static void _get_bones_mat(PiMatrix4 *mat, BoneTransform *bones, PiSkeleton *sk);

/* 取骨骼对应骨头的数目 */
static uint32 _get_bones_num(PiSkeleton *sk);

/* 拷贝初始姿态的逆到目的动画 */
static void _copy_inv_initpose_to_anim(PiSkeleton *dst, PiSkeleton *src);

/* 结束时处理 */
static void _end_play_handle(PiController *c)
{
	SkeletonAnimate *impl = (SkeletonAnimate *)(c->impl);

	BoneAnimaInfo* nextAnimInfo = pi_queue_pop_head(impl->animQueue);
	if (nextAnimInfo != NULL)
	{
		if (nextAnimInfo->defaultName)
		{
			if (impl->defaultName)
			{
				pi_free(impl->defaultName);
			}
			impl->defaultName = pi_str_dup(nextAnimInfo->defaultName);
		}
		pi_skanim_play(c, nextAnimInfo->name, nextAnimInfo->transient_time, nextAnimInfo->speed, nextAnimInfo->state, nextAnimInfo->start_time, FALSE);
		_free_anim_info(nextAnimInfo);
		return;
	}
	if(impl->state == AS_LOOP)
	{
		impl->is_stop = FALSE;
	}
	else if(impl->state == AS_KEEP)
	{
		impl->is_stop = TRUE;

		impl->curr_time = 0.0f;
		impl->start_time = 0.0f;
		impl->transient_time = 0.0f;
		
		impl->play_anim = NULL;
		impl->last_anim = NULL;
	}
	else if (impl->state == AS_DEFAULT)
	{
		if (impl->defaultName == NULL)
		{
			impl->is_stop = TRUE;
		}
		else
		{
			impl->is_stop = FALSE;
			pi_skanim_play(c, impl->defaultName, impl->transient_time, impl->speed, AS_LOOP, 0.0, FALSE);
		}
	}
	else
	{
		pi_skanim_stop(c);
	}
}

/* 取动画播放的权重 */
static float _get_play_weight(SkeletonAnimate *impl, float play_time)
{
	float weight = 1.0f;

	if(impl->transient_time > 0 && play_time < impl->transient_time)
	{
		weight = play_time / impl->transient_time;
	}
	else
	{
		impl->last_anim = NULL;
		impl->start_time = 0.0f;
		impl->transient_time = 0.0f;
	}

	return weight;
}

static void  _compute_matrix(SkeletonAnimate *impl, float real_time, PiBool enable_fade)
{
	uint i;
	_anim_bones(impl->bones, impl->play_anim, (uint)real_time);

	/* 动画的淡入淡出 */
	if (impl->last_anim != NULL && impl->transient_time > 0.0001f && enable_fade)
	{
		float weight = _get_play_weight(impl, real_time);
		_blend_bones(impl->bones, impl->last_bones, impl->bones, weight);
	}

	/* 计算骨骼矩阵 */
	_get_bones_mat(impl->bone_mats, impl->bones, impl->play_anim);

	/* 拷贝矩阵的前3行 */
	for (i = 0; i < impl->num_bones; ++i)
	{
		pi_memcpy_inline(impl->bone_mat_columns + i * 12, (void *)impl->bone_mats[i].m, 12 * sizeof(float));
	}
}

static PiBool _update_key_frame_animation(SkeletonAnimate *impl, PiController *c, float tpf)
{
	float real_time;

	if (impl->is_stop)
	{
		/* 动画已经停止 */
		BoneAnimaInfo* nextAnimInfo = pi_queue_pop_head(impl->animQueue);
		if (nextAnimInfo != NULL)
		{
			if (nextAnimInfo->defaultName)
			{
				if (impl->defaultName != NULL)
				{
					pi_free(impl->defaultName);
				}
				impl->defaultName = pi_str_dup(nextAnimInfo->defaultName);
			}
			pi_skanim_play(c, nextAnimInfo->name, nextAnimInfo->transient_time, nextAnimInfo->speed, nextAnimInfo->state, nextAnimInfo->start_time, FALSE);
			_free_anim_info(nextAnimInfo);
		}
		else
		{
			return FALSE;
		}
	}



	if (impl->num_bones > 0)
	{
		// 当前时间
		impl->curr_time += 1000 * tpf * impl->speed;

		real_time = impl->curr_time - impl->start_time;

		// 超出播放时间，而且不是循环状态，保留最大时间
		if (real_time >= impl->play_time && impl->state != AS_LOOP)
		{
			real_time = (float)impl->play_time;
		}

		if (impl->is_cache_anim)
		{

			//计算当前帧ID
			uint frame_id = (uint)(real_time / frame_delta_time);

			void* matrix_buffer = pi_skeleton_get_cache_matrix(impl->play_anim, frame_id);
			if (matrix_buffer == NULL)
			{
				_compute_matrix(impl, frame_id % pi_skanim_get_cache_num(impl->play_anim) * frame_delta_time, FALSE);
				impl->cach_matrix_impl = pi_skeleton_set_cache_matrix(impl->play_anim, frame_id, impl->bone_mat_columns);
			}
			else
			{
				impl->cach_matrix_impl = matrix_buffer;
			}
		}
		else
		{
			_compute_matrix(impl, real_time, TRUE);
			impl->cach_matrix_impl = impl->bone_mat_columns;
		}
		/* 超时整理 */
		if (real_time >= impl->play_time)
		{
			_end_play_handle(c);
		}
	}
	return TRUE;
}

static PiBool _update_ragdoll(SkeletonAnimate* impl, PiController* c)
{
	uint i;
	PI_ASSERT(impl->ragdoll != NULL, "has not been attached to any ragdoll");

	_init_bones(impl->sk, impl->bones);


	pi_physics_ragdoll_bind_bone_pose(impl->ragdoll, impl->bones);
	
	_bind_bones(impl->bones, impl->play_anim != NULL ? impl->play_anim : impl->sk);
	/* 计算骨骼矩阵 */
	_get_bones_mat(impl->bone_mats, impl->bones, impl->play_anim != NULL ? impl->play_anim : impl->sk);

	/* 拷贝矩阵的前3行 */
	for (i = 0; i < impl->num_bones; ++i)
	{
		pi_memcpy_inline(impl->bone_mat_columns + i * 12, (void *)impl->bone_mats[i].m, 12 * sizeof(float));
	}
	return TRUE;
}


static PiBool _update(PiController *c, float tpf)
{
	SkeletonAnimate *impl = (SkeletonAnimate *)(c->impl);
	PiBool r = FALSE;

	if (impl->type == SKT_ANIM)
	{
		r = _update_key_frame_animation(impl, c, tpf);
	}
	else
	{
		r = _update_ragdoll(impl, c);
	}
	return r;
}

static PiBool _apply(PiController *c, ControllerApplyType type, void *obj)
{
	PiEntity *entity;
	PiMesh* mesh;
	PiMaterial *material;
	SkeletonAnimate *impl = (SkeletonAnimate *)(c->impl);
	
	PI_ASSERT(type == CAT_ENTITY, "apply skanim controller failed, accept entity type, type = %d", type);
	entity = (PiEntity *)obj;
	material = entity->material;
	mesh = entity->mesh->mesh;
	if(impl->num_bones > 0)
	{
		//如果骨骼数量超过60根，采样软件蒙皮
		if (mesh->version > 2)
		{
			impl->bone_info = mesh->data.bone_num;
		}
		else
		{
			impl->bone_info = impl->num_bones;
		}

		if (impl->bone_info > 60)
		{
			pi_material_set_def(material, impl->HARDWARE_SKELETON, FALSE);
			pi_log_print(LOG_INFO, "骨骼数量超过60根");
		}
		else{
			pi_material_set_def(material, impl->HARDWARE_SKELETON, TRUE);
			pi_material_set_def(material, "RAGDOLL", impl->type == SKT_RAGDOLL);
			/*理论上bone_offset应该做一次打包才能传入，这里由于offset后面有足够的空间，所以省去了*/
			pi_material_set_uniform_pack_flag(material, impl->U_boneInfo, UT_FLOAT, 1, &mesh->data.bone_offset, TRUE, TRUE);
			if (impl->is_cache_anim && impl->type == SKT_ANIM){
				pi_material_set_uniform_pack_flag(material, impl->U_boneMatrices, UT_MATRIX4x3, impl->bone_info, impl->cach_matrix_impl + (int)(mesh->data.bone_offset * 3 * 4), FALSE, TRUE);
			}
			else
			{
				pi_material_set_uniform_pack_flag(material, impl->U_boneMatrices, UT_MATRIX4x3, impl->bone_info, impl->bone_mat_columns + (int)(mesh->data.bone_offset * 3 * 4), FALSE, TRUE);
			}
		}
	}
	else
	{
		pi_material_set_def(material, impl->HARDWARE_SKELETON, FALSE);
	}
	return TRUE;
}

PiController *PI_API pi_skanim_new()
{
	SkeletonAnimate *impl = pi_new0(SkeletonAnimate, 1);
	PiController *c = pi_controller_new(CT_SKANIM, _apply, _update, impl);
	impl->is_stop = TRUE;
	
	impl->HARDWARE_SKELETON = pi_conststr("HARDWARE_SKELETON");
	impl->RAGDOLL = pi_conststr("RAGDOLL");
	impl->U_boneMatrices = pi_conststr("u_boneMatrices");
	impl->U_boneInfo = pi_conststr("u_boneInfo");
	impl->animQueue = pi_queue_new();

	pi_hash_init(&impl->anim_map, 0.75f, pi_str_hash, pi_string_equal);
	pi_queue_init(impl->animQueue);
	return c;
}

static PiSelectR PI_API _delete_key(void *user_data, PiKeyValue  *value)
{
	pi_free(value->key);
		return SELECT_NEXT;
}
static PiSelectR PI_API _delete_queue_key(void* user_data, void* value)
{
	_free_anim_info(value);
	return SELECT_NEXT;
}
void PI_API pi_skanim_free(PiController *c)
{
	if (c != NULL)
	{
		SkeletonAnimate *impl = (SkeletonAnimate *)(c->impl);

		pi_hash_foreach(&impl->anim_map, _delete_key, NULL);

		pi_hash_clear(&impl->anim_map, TRUE);

		if (impl->bone_mats != NULL)
			pi_free(impl->bone_mats);
		if (impl->bone_mat_columns != NULL)
			pi_free(impl->bone_mat_columns);
		_destroy_bones(impl->bones);
		_destroy_bones(impl->last_bones);
		if (impl->animQueue)
		{
			pi_queue_foreach(impl->animQueue, _delete_queue_key, NULL);
			pi_queue_free(impl->animQueue);
		}
		if (impl->defaultName)
		{
			pi_free(impl->defaultName);
		}
		pi_free(impl);
		pi_controller_free(c);
	}	
}

void PI_API pi_skamin_set_flag(PiController *c, PiBool cacheFlag)
{
	SkeletonAnimate *impl = (SkeletonAnimate*)(c->impl);
	impl->is_cache_anim = cacheFlag;
}

PiBool PI_API pi_skanim_set_skeleton(PiController *c, PiSkeleton *sk)
{
	uint32 i;
	SkeletonAnimate *impl = (SkeletonAnimate *)(c->impl);

	/* 只允许设置一次，因为在skbinding中需要依赖该骨骼的某个骨头id */
	PI_ASSERT(impl->sk == NULL, "only set skeleton once");

	impl->sk = sk;

	if(impl->bone_mats != NULL)
	{
		_destroy_bones(impl->bones);
		_destroy_bones(impl->last_bones);
		pi_free(impl->bone_mats);
		pi_free(impl->bone_mat_columns);
	}

	impl->num_bones = _get_bones_num(impl->sk);
	impl->bone_mats = pi_new0(PiMatrix4, impl->num_bones);
	impl->bone_mat_columns = pi_new0(float, 3 * 4 * impl->num_bones);
	impl->cach_matrix_impl = impl->bone_mat_columns;
	for(i = 0; i < impl->num_bones; ++i)
	{
		float *addr = impl->bone_mat_columns + i * 12;
		pi_memset_inline(addr, 0, 3 * 4 * sizeof(float));
		addr[0] = addr[5] = addr[10] = 1.0f;

		pi_mat4_set_identity(&impl->bone_mats[i]);
	}

	impl->bones = _create_bones(sk);
	impl->last_bones = _create_bones(sk);

	return TRUE;
}

PiBool PI_API pi_skanim_add(PiController *c, const char *name, PiSkeleton *anim)
{
	PiKeyValue old;
	SkeletonAnimate *impl = (SkeletonAnimate *)(c->impl);
	char *anim_name = pi_str_dup(name);

	if(pi_hash_enter(&impl->anim_map, anim_name, anim, &old))
	{
		pi_free(old.key);
	}

	return TRUE;
}

PiBool PI_API pi_skanim_remove(PiController *c, const char *name)
{
	PiKeyValue old;
	SkeletonAnimate *impl = (SkeletonAnimate *)(c->impl);

	if(pi_hash_delete(&impl->anim_map, name, &old))
	{
		pi_free(old.key);
	}

	return TRUE;
}

PiBool PI_API pi_skanim_add_anim_to_queue(PiController *c, const char *name, float transient_time, float speed, AnimationState state, float start_time, const char* defaultName)
{
	SkeletonAnimate *impl = (SkeletonAnimate*)(c->impl);
	BoneAnimaInfo* info = _create_anim_info();
	info->name = pi_str_dup(name);
	if (defaultName)
	{
		info->defaultName = pi_str_dup(defaultName);
	}

	info->speed = speed;
	info->start_time = start_time;
	info->state = state;
	info->transient_time = transient_time;
	pi_queue_push_tail(impl->animQueue, info);
	return TRUE;
}

void PI_API pi_skanim_update_speed(PiController *c, float speed)
{
	SkeletonAnimate *impl = (SkeletonAnimate *)(c->impl);
	if (impl == NULL) {
		return;
	}
	impl->speed = speed;
}


PiBool PI_API pi_skanim_play(PiController *c, const char *name,
                             float transient_time, float speed, AnimationState state, float start_time, PiBool is_clear_queue)
{
	PiSkeleton *anim = NULL;
	SkeletonAnimate *impl = (SkeletonAnimate *)(c->impl);
	PiBool r = (impl->sk != NULL);
	r = r && pi_hash_lookup(&impl->anim_map, name, (void **)(&anim));

	if(r)
	{
		uint num;

		if(impl->play_anim != NULL && impl->bones->is_computed[0] != FALSE)
		{
			/* 记录上一次播放的动画 */
			impl->last_anim = impl->play_anim;
			_copy_bones(impl->last_bones, impl->bones);
		}
		else
		{
			impl->last_anim = NULL;
		}

		/* 这一次播放的动画信息 */
		impl->is_stop = FALSE;
		impl->curr_time = start_time * 1000.0f;
		impl->start_time = 0.0f;
		impl->speed = speed;
		impl->state = state;
		impl->play_anim = anim;
		impl->transient_time = 0.0f;

		impl->play_time = pi_skeleton_get_animation_time(impl->play_anim);

		if(impl->last_anim != NULL)
		{
			impl->transient_time = 1000.0f * transient_time;
		}

		/* 如果需要清除动画列表，需要把列表清除*/
		if (is_clear_queue && impl->animQueue)
		{
			pi_queue_foreach(impl->animQueue, _delete_queue_key, NULL);
			pi_queue_clear(impl->animQueue, FALSE);
		}

		/* 将原始骨骼数据放到动画 */
		_copy_inv_initpose_to_anim(impl->play_anim, impl->sk);

		num = _get_bones_num(impl->play_anim);
		PI_ASSERT(num == impl->num_bones, "different animation, bone's num isn't same");
		c->delta_time = 0;
	}

	return r;
}

PiBool PI_API pi_skanim_stop(PiController *c)
{
	uint i;
	SkeletonAnimate *impl = (SkeletonAnimate *)(c->impl);

	if(impl->is_stop)
	{
		return TRUE;
	}

	switch(impl->state)
	{
		case AS_KEEP:
			_anim_bones(impl->bones, impl->play_anim, impl->play_time);
			/* 计算骨骼矩阵 */
			_get_bones_mat(impl->bone_mats, impl->bones, impl->play_anim);

			/* 拷贝矩阵的前3行 */
			for(i = 0; i < impl->num_bones; ++i)
			{
				pi_memcpy_inline(impl->bone_mat_columns + i * 12, (void *)impl->bone_mats[i].m, 12 * sizeof(float));
			}

			break;

		case AS_FIRST:
			_anim_bones(impl->bones, impl->play_anim, 0);
			/* 计算骨骼矩阵 */
			_get_bones_mat(impl->bone_mats, impl->bones, impl->play_anim);

			/* 拷贝矩阵的前3行 */
			for(i = 0; i < impl->num_bones; ++i)
			{
				pi_memcpy_inline(impl->bone_mat_columns + i * 12, (void *)impl->bone_mats[i].m, 12 * sizeof(float));
			}

			break;

		default:
			for(i = 0; i < impl->num_bones; ++i)
			{
				/* 复原为单位阵，还原到默认状态 */
				float *addr = impl->bone_mat_columns + i * 12;
				pi_memset_inline(addr, 0, 3 * 4 * sizeof(float));
				addr[0] = addr[5] = addr[10] = 1.0f;

				pi_mat4_set_identity(&impl->bone_mats[i]);
			}

			break;
	}

	impl->curr_time = 0.0f;
	impl->start_time = 0.0f;
	impl->transient_time = 0.0f;

	impl->play_anim = NULL;
	impl->last_anim = NULL;

	impl->is_stop = TRUE;

	return TRUE;
}

sint PI_API pi_skanim_get_bone_id(PiController *c, const char *bone_name)
{
	SkeletonAnimate *impl = (SkeletonAnimate *)(c->impl);

	if(impl->sk != NULL)
	{
		uint32 i;
		uint32 num = impl->sk->boneData.elemNum;
		Bone *bones = (Bone *)(impl->sk->boneData.data);
		
		wchar *w_name = pi_utf8_to_wstr(bone_name);
		for(i = 0; i < num; ++i)
		{
			if (pi_wstr_compare(bones[i].name, w_name) == 0)
			{
				pi_free(w_name);
				return i;
			}
		}
		
		pi_free(w_name);
		PI_ASSERT(i == num, "skanim, get bone matrix, can't find the boen, name = %s", bone_name);
	}

	return -1;
}

PiBool PI_API pi_skanim_get_bone_matrix(PiController *c, uint bone_id, PiMatrix4 *dst)
{
	SkeletonAnimate *impl = (SkeletonAnimate *)(c->impl);

	PI_ASSERT(bone_id < impl->bones->num, "bone_id is out of range");
	
	_get_init_bone_matrix(impl->sk, dst, bone_id);
	if (impl->is_cache_anim && impl->play_anim != NULL)
	{
		PiMatrix4 m;
		pi_memcpy_inline(&m, impl->cach_matrix_impl + 12 * bone_id, 12 * sizeof(float));
		pi_math_vec4_set((PiVector4*)m.m[3], 0, 0, 0, 1);
		pi_mat4_mul(dst, &m, dst);
	}
	else{
		pi_mat4_mul(dst, &impl->bone_mats[bone_id], dst);
	}
	
	return TRUE;
}


static BoneAnimaInfo* _create_anim_info()
{
	BoneAnimaInfo* info = pi_new0(BoneAnimaInfo, 1);
	return info;
}
static void _free_anim_info(BoneAnimaInfo* info)
{
	if (info->name)
	{
		pi_free(info->name);
	}
	if (info->defaultName)
	{
		pi_free(info->defaultName);
	}
	pi_free(info);
}

/* 创建骨头数据 */
static BoneTransform *_create_bones(PiSkeleton *sk)
{
	BoneTransform *bones = pi_new0(BoneTransform, 1);

	bones->num = sk->boneData.elemNum;
	bones->transforms = pi_new0(TransformData, bones->num);
	bones->is_computed = pi_new0(PiBool, bones->num);

	return bones;
}

/* 释放骨头数据 */
static void _destroy_bones(BoneTransform *bones)
{
	if (bones != NULL)
	{
		pi_free(bones->transforms);
		pi_free(bones->is_computed);
		pi_free(bones);
	}	
}

/* 拷贝骨头数据 */
static void _copy_bones(BoneTransform *dst, BoneTransform *src)
{
	PI_ASSERT(dst->num == src->num, "copy failed, bones's num is different");
	pi_memcpy_inline(dst->is_computed, src->is_computed, src->num * sizeof(PiBool));
	pi_memcpy_inline(dst->transforms, src->transforms, src->num * sizeof(TransformData));
}

/* 克隆骨头数据 */
static BoneTransform *_clone_bones(BoneTransform *bones)
{
	BoneTransform *dst = pi_new0(BoneTransform, 1);
	_copy_bones(dst, bones);
	return dst;
}

/* 骨头数据设为初始姿态 */
static void _init_bones(PiSkeleton *sk, BoneTransform *bones)
{
	uint i;
	Bone *src = (Bone *)sk->boneData.data;
	pi_memset_inline(bones->is_computed, FALSE, sizeof(PiBool) * sk->boneData.elemNum);

	for(i = 0; i < bones->num; ++i)
	{
		pi_memcpy_inline(&bones->transforms[i], &src[i].localData, sizeof(TransformData));
	}
}

static void _get_init_bone_matrix(PiSkeleton *sk, PiMatrix4 *dst, uint bone_id)
{
	TransformData transData;
	TransformData *pData = &sk->invBindPose[bone_id];
	pi_vec3_set(&transData.translate, -pData->translate.x, -pData->translate.y, -pData->translate.z);
	pi_vec3_set(&transData.scale, 1.0f / pData->scale.x, 1.0f / pData->scale.y, 1.0f / pData->scale.z);
	PI_ASSERT(pi_quat_inverse(&transData.rotate, &pData->rotate), "rotate error");
	pi_mat4_build_transform(dst, &transData.translate, &transData.scale, &transData.rotate);
}

static void _keyframe_interpolate(AnimKeyFrame *dst, AnimTrack *pTrack, uint32 keyTime)
{
	uint32 i;
	int lastId = 0, nextId = 0;	/* 上一个与下一个帧的ID */
	AnimKeyFrame *pKeyFrame = pTrack->keyframes.data;

	dst->startTime = keyTime;

	/* 查找开始时间介于keyTime的前后帧 */
	for (i = 0; i < pTrack->keyframes.elemNum; ++i)
	{
		if (keyTime <= pKeyFrame[i].startTime)
		{
			lastId = i - 1;
			nextId = i;

			if (lastId < 0)
			{
				lastId = 0;
				nextId = lastId + 1;
			}

			break;
		}
	}
	if (interpolate)
	{
		float frac = 0.0f;		/* 插值百分比 */
		/* 计算插值百分比 */
		frac = 1.0f *
		       (keyTime - pKeyFrame[lastId].startTime) / (pKeyFrame[nextId].startTime - pKeyFrame[lastId].startTime);

		/* 旋转采用线性插值 */
		pi_quat_lerp(&dst->transform.rotate,
		             &pKeyFrame[lastId].transform.rotate, &pKeyFrame[nextId].transform.rotate, frac, TRUE);

		/* 平移采用线性插值 */
		pi_vec3_lerp(&dst->transform.translate,
		             &pKeyFrame[lastId].transform.translate, &pKeyFrame[nextId].transform.translate, frac);

		/* 缩放采用线性插值 */
		pi_vec3_lerp(&dst->transform.scale,
		             &pKeyFrame[lastId].transform.scale, &pKeyFrame[nextId].transform.scale, frac);
	}
	else
	{
		pi_memcpy_inline(&dst->transform, &pKeyFrame[lastId].transform, sizeof(TransformData));
	}
}

static void _transform_apply_keyframe(TransformData *dst, TransformData *src, AnimKeyFrame *keyFrame)
{
	pi_vec3_add(&dst->translate, &src->translate, &keyFrame->transform.translate);
	pi_quat_mul(&dst->rotate, &src->rotate, &keyFrame->transform.rotate);
	pi_vec3_mul(&dst->scale, &src->scale, &keyFrame->transform.scale);
}

static void _bones_on_anim(BoneTransform *bones, Animation *anim, uint time)
{
	uint32 i;
	AnimTrack *tracks = (AnimTrack *)anim->tracks.data;

	if(anim->playTime <= 0)
	{
		time = 0;
	}
	else if(time > anim->playTime)
	{
		time %= anim->playTime;
	}

	for(i = 0; i < anim->tracks.elemNum; ++i)
	{
		AnimKeyFrame keyFrame;
		uint32 bone_id = tracks[i].boneId;
		PI_ASSERT(bone_id < bones->num, "animation use out-of-range bone");
		_keyframe_interpolate(&keyFrame, tracks + i, time);
		_transform_apply_keyframe(&bones->transforms[bone_id], &bones->transforms[bone_id], &keyFrame);
	}
}

/**
 * 将动画作用于骨头
 * time: 动画时间，单位：毫秒
 */
static void _anim_bones(BoneTransform *bones, PiSkeleton *sk, uint time)
{
	Animation *anim = (Animation *)sk->animData.data;
	PI_ASSERT(anim != NULL, "compute animation failed, animation isn't exist");

	_init_bones(sk, bones);

	_bones_on_anim(bones, anim, time);

	_bind_bones(bones, sk);
}

/* 骨头混合 */
static void _blend_bones(BoneTransform *dst, BoneTransform *src1, BoneTransform *src2, float weight)
{
	uint i;
	TransformData *tr_dst = dst->transforms;
	TransformData *tr_src1 = src1->transforms;
	TransformData *tr_src2 = src2->transforms;

	for(i = 0; i < dst->num; ++i)
	{
		pi_vec3_lerp(&tr_dst[i].scale, &tr_src1[i].scale, &tr_src2[i].scale, weight);
		pi_vec3_lerp(&tr_dst[i].translate, &tr_src1[i].translate, &tr_src2[i].translate, weight);
		pi_quat_lerp(&tr_dst[i].rotate, &tr_src1[i].rotate, &tr_src2[i].rotate, weight, TRUE);
	}
}

// 计算当前的绑定姿态
static void _bind_bone_pose(BoneTransform *bone_trs, Bone *bone_data, uint bone_id)
{
	uint32 parent_id = bone_data[bone_id].parentID;
	TransformData *pData = &bone_trs->transforms[bone_id];
	
	PI_ASSERT(parent_id < bone_trs->num, "parent_id is out-of-range");
	
	if(bone_trs->is_computed[bone_id])
	{
		return;
	}

	if(bone_id != parent_id)
	{
		/* 根据父骨头数据计算本骨头的数据 */

		TransformData *parent = &bone_trs->transforms[parent_id];

		if(!bone_trs->is_computed[parent_id])
		{
			_bind_bone_pose(bone_trs, bone_data, parent_id);
		}

		pi_vec3_mul(&pData->scale, &parent->scale, &pData->scale);

		pi_quat_mul(&pData->rotate, &parent->rotate, &pData->rotate);

		pi_vec3_mul(&pData->translate, &parent->scale, &pData->translate);
		pi_quat_rotate_vec3(&pData->translate, &pData->translate, &parent->rotate);
		pi_vec3_add(&pData->translate, &pData->translate, &parent->translate);
	}

	bone_trs->is_computed[bone_id] = TRUE;
}

static void _compute_bone_mat(PiMatrix4 *mat, TransformData *inv_init_pose, TransformData *tr)
{
	PiVector3 locScale;
	PiVector3 locTranslate;
	PiQuaternion locRotate;

	pi_vec3_mul(&locScale, &tr->scale, &inv_init_pose->scale);
	pi_quat_mul(&locRotate, &tr->rotate, &inv_init_pose->rotate);

	pi_vec3_mul(&locTranslate, &locScale, &inv_init_pose->translate);
	pi_quat_rotate_vec3(&locTranslate, &locTranslate, &locRotate);
	pi_vec3_add(&locTranslate, &tr->translate, &locTranslate);

	pi_mat4_build_transform(mat, &locTranslate, &locScale, &locRotate);
}

/* 取到骨头的姿态 */
static void _bind_bones(BoneTransform *bones, PiSkeleton *sk)
{
	uint i;

	for(i = 0; i < bones->num; ++i)
	{
		if(!bones->is_computed[i])
		{
			_bind_bone_pose(bones, sk->boneData.data, i);
		}
	}
}

/* 取到骨骼矩阵 */
static void _get_bones_mat(PiMatrix4 *mat, BoneTransform *bones, PiSkeleton *sk)
{
	uint i;

	for(i = 0; i < bones->num; ++i)
	{
		_compute_bone_mat(&mat[i], &sk->invBindPose[i], &bones->transforms[i]);
	}
}

static uint32 _get_bones_num(PiSkeleton *sk)
{
	uint32 num = 0;

	if(sk != NULL)
	{
		num = sk->boneData.elemNum;
	}

	return num;
}

static void _copy_inv_initpose_to_anim(PiSkeleton *dst, PiSkeleton *src)
{
	uint32 numBone = 0;

	if(dst->invBindPose == NULL || src->invBindPose == NULL)
		PI_ASSERT(FALSE, "invBindPos is NULL");

	numBone = (dst->boneData.elemNum < src->boneData.elemNum) ? dst->boneData.elemNum : src->boneData.elemNum;

	pi_memcpy_inline(dst->invBindPose, src->invBindPose, sizeof(TransformData) * numBone);
}

PiBool PI_API pi_skanim_attach_ragdoll(PiController *c, Ragdoll* ragdoll)
{
	SkeletonAnimate *impl = (SkeletonAnimate *)(c->impl);
	impl->ragdoll = ragdoll;
	return TRUE;
}

void PI_API pi_skanim_detach_ragdoll(PiController* c)
{
	SkeletonAnimate* impl = (SkeletonAnimate*)(c->impl);
	pi_skanim_change_status(c, SKT_ANIM);
	impl->ragdoll = NULL;
}

void PI_API pi_skanim_change_status(PiController *c, SkeletonAnimType type)
{
	SkeletonAnimate *impl = (SkeletonAnimate*)(c->impl);
	impl->type = type;
	if (SKT_RAGDOLL == type)
	{

		pi_physics_ragdoll_init_bone(impl->ragdoll, c);
		pi_physics_ragdoll_set_type(impl->ragdoll, RT_Dynamic);
	}
	else
	{
		pi_physics_ragdoll_set_type(impl->ragdoll, RT_Kinematic);
	}
}

SkeletonAnimType PI_API pi_skanim_get_status(PiController* c)
{
	SkeletonAnimate *impl = (SkeletonAnimate*)(c->impl);
	return impl->type;
}