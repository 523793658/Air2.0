#include <pi_skeleton.h>

#define SKELETON_VERSION "TENGINE_SKEL_V01"
#define SKELETON_HEADER_LEN sizeof(SKELETON_VERSION) - 1 + sizeof(uint32)
#define MAX_BONES 60

static void bufferdata_set(BufferData *buf, uint32 elemSize, uint32 elemNum, void *data)
{
	buf->data = data;
	buf->elemNum = elemNum;
	buf->elemSize = elemSize;
}

static void bufferdata_copy(BufferData *buf, uint32 elemSize, uint32 elemNum, void *data)
{
	void *temp = NULL;
	pi_memset_inline(buf, 0, sizeof(BufferData));
	if(0 < elemNum && 0 < elemSize)
	{
		uint32 size = elemNum * elemSize;
		temp = pi_malloc(size);
		if(NULL != data)
			pi_memcpy_inline(temp, data, size);
	}
	bufferdata_set(buf, elemSize, elemNum, temp);
}

static void bufferdata_free(BufferData *buf)
{
	if(NULL != buf->data)
		pi_free(buf->data);
}

static PiBool check_sk_header(byte *data, uint32 size)
{
	uint32 len = 0;
	uint32 flagLen = sizeof(SKELETON_VERSION) - 1;
	if(size < SKELETON_HEADER_LEN)
		return FALSE;
	if(0 != pi_memcmp_inline(data, SKELETON_VERSION, flagLen))
		return FALSE;
	len = *(uint32*)(data + flagLen);
	if(len != size)
		return FALSE;
	return TRUE;
}

static void bone_load(BufferData *boneData, PiBytes *bb)
{
	short numBone = 0;
	Bone *pBone = NULL;
	unsigned short i = 0;
	char *name = NULL;

	pi_bytes_read_int16(bb, &numBone);
	//PI_ASSERT(numBone <= MAX_BONES, "%s: %d, %s%d", "too many bones", numBone, "keep bone number under ", MAX_BONES);
	bufferdata_copy(boneData, sizeof(Bone), numBone, NULL);
	pBone = boneData->data;
	for(i = 0; i < numBone; ++i)
	{
		short id = 0;
		PiVector3 axis;
		int8 hasScale = 0;
		float radAng = 0.0f;		

		/* 该骨头的id */
		pi_bytes_read_int16(bb, &id);
		PI_ASSERT(id < numBone, "骨骼导入，骨头ID数据不对");

		pi_memset_inline(&pBone[id], 0, sizeof(pBone[id]));


		pi_bytes_read_str(bb, &name);
		pBone[id].name = pi_str_to_wstr(name, PI_CP_UTF8);

		/* 父骨头的ID */
		pi_bytes_read_int16(bb, &pBone[id].parentID);

		/* 位移 */
		pi_bytes_read_float(bb, &pBone[id].localData.translate.x);
		pi_bytes_read_float(bb, &pBone[id].localData.translate.y);
		pi_bytes_read_float(bb, &pBone[id].localData.translate.z);

		/* 旋转 */
		pi_bytes_read_float(bb, &radAng);
		pi_bytes_read_float(bb, &axis.x);
		pi_bytes_read_float(bb, &axis.y);
		pi_bytes_read_float(bb, &axis.z);
		pi_quat_from_angle_axis(&pBone[id].localData.rotate, &axis, radAng);

		/* 缩放 */
		pi_bytes_read_int8(bb, &hasScale);
		if(0 == hasScale)
			pi_vec3_copy(&pBone[id].localData.scale, pi_vec3_get_scale_unit());
		else
		{
			pi_bytes_read_float(bb, &pBone[id].localData.scale.x);
			pi_bytes_read_float(bb, &pBone[id].localData.scale.y);
			pi_bytes_read_float(bb, &pBone[id].localData.scale.z);
		}
	}
}

static void free_bone_data(BufferData *boneData)
{
	Bone *pBone = NULL;
	unsigned short i = 0;
	pBone = boneData->data;
	for (i = 0; i < boneData->elemNum; ++i)
	{
		if (pBone[i].name != NULL)
		{
			pi_free(pBone[i].name);
		}
	}
	bufferdata_free(boneData);
}

static void anim_keyframe_load(BufferData *frameData, PiBytes *bb)
{
	short i = 0;
	short numFrame = 0;
	AnimKeyFrame *pKeyFrame = NULL;
	pi_bytes_read_int16(bb, &numFrame);
	bufferdata_copy(frameData, sizeof(AnimKeyFrame), numFrame, NULL);
	pKeyFrame = frameData->data;
	for(i = 0; i < numFrame; ++i)
	{
		PiVector3 axis;
		int8 hasScale = 0;
		float radAng = 0.0f;
		sint startTime = 0;

		pi_bytes_read_int(bb, &startTime);
		pKeyFrame[i].startTime = startTime;

		pi_bytes_read_float(bb, &pKeyFrame[i].transform.translate.x);
		pi_bytes_read_float(bb, &pKeyFrame[i].transform.translate.y);
		pi_bytes_read_float(bb, &pKeyFrame[i].transform.translate.z);

		pi_bytes_read_float(bb, &radAng);
		pi_bytes_read_float(bb, &axis.x);
		pi_bytes_read_float(bb, &axis.y);
		pi_bytes_read_float(bb, &axis.z);
		pi_quat_from_angle_axis(&pKeyFrame[i].transform.rotate, &axis, radAng);

		pi_bytes_read_int8(bb, &hasScale);
		if(0 == hasScale)
			pi_vec3_copy(&pKeyFrame[i].transform.scale, pi_vec3_get_scale_unit());
		else
		{
			pi_bytes_read_float(bb, &pKeyFrame[i].transform.scale.x);
			pi_bytes_read_float(bb, &pKeyFrame[i].transform.scale.y);
			pi_bytes_read_float(bb, &pKeyFrame[i].transform.scale.z);
		}
	}
}

static void anim_track_load(BufferData *trackData, PiBytes *bb)
{
	short i = 0;
	short numTrack = 0;
	AnimTrack *pTrack = NULL;
	pi_bytes_read_int16(bb, &numTrack);
	bufferdata_copy(trackData, sizeof(AnimTrack), numTrack, NULL);
	pTrack = trackData->data;
	for(i = 0; i < numTrack; ++i)
	{
		pi_bytes_read_int16(bb, &pTrack[i].boneId);
		anim_keyframe_load(&pTrack[i].keyframes, bb);
	}
}

static void anim_load(BufferData *animData, PiBytes *bb)
{
	short i = 0;
	short numAnim = 0;
	Animation *pAnim = NULL;
	pi_bytes_read_int16(bb, &numAnim);
	bufferdata_copy(animData, sizeof(Animation), numAnim, NULL);
	pAnim = animData->data;
	for(i = 0; i < numAnim; ++i)
	{
		char *name;
		sint playTime = 0;
		pi_bytes_read_str(bb, &name);
		pAnim[i].name = pi_str_to_wstr(name, PI_CP_UTF8);
		pi_bytes_read_int(bb, &playTime);
		pAnim[i].playTime = playTime;
		pi_memset_inline(&pAnim[i].matrix_cache, 0, sizeof(BufferData));
		anim_track_load(&pAnim[i].tracks, bb);
	}
}

static void sk_load_impl(PiSkeleton *skeleton, PiBytes *bb)
{
	/* 根骨头的id */

	//其实不需要rootID
	int16 rootID = 0;
	pi_bytes_read_int16(bb, &rootID);

	/* 骨头数据 */
	bone_load(&skeleton->boneData, bb);
	/* 动画数据 */
	anim_load(&skeleton->animData, bb);
}

/* 计算每根骨头的逆元素 */
static PiBool skeleton_compute_inv_pose(TransformData *invArray, TransformData *bindingArray, uint32 num)
{
	uint32 i;
	for(i = 0; i < num; ++i)
	{
		TransformData *data = bindingArray + i;
		pi_vec3_set(&invArray[i].translate, -data->translate.x, -data->translate.y, -data->translate.z);
		pi_vec3_set(&invArray[i].scale, 1.0f/data->scale.x, 1.0f/data->scale.y, 1.0f/data->scale.z);
		if(!pi_quat_inverse(&invArray[i].rotate, &data->rotate))
			return FALSE;
	}
	return TRUE;
}

static void bone_binding_pose(TransformData *bindArray, Bone *bones, PiBool *isCompute, uint32 id)
{
	uint32 parentID = bones[id].parentID;
	TransformData *pData = &bindArray[id];
	if(isCompute[id])
		return;

	if(id != parentID)
	{/* 根据父骨头数据计算本骨头的数据 */
		PiVector3 scalePos;
		TransformData *parentData = &bindArray[parentID];
		if(!isCompute[parentID])
			bone_binding_pose(bindArray, bones, isCompute, parentID);
		pi_vec3_mul(&pData->scale, &parentData->scale, &pData->scale);
		pi_quat_mul(&pData->rotate, &parentData->rotate, &pData->rotate);

		pi_vec3_mul(&scalePos, &parentData->scale, &pData->translate);
		pi_quat_rotate_vec3(&pData->translate, &scalePos, &parentData->rotate);

		pi_vec3_add(&pData->translate, &pData->translate, &parentData->translate);
	}
	isCompute[id] = TRUE;
}

/* 从二进制数据data创建骨骼数据 */
PiSkeleton* PI_API pi_skeleton_new(byte *data, uint32 size)
{
	PiSkeleton *sk = pi_new0(PiSkeleton, 1);
	pi_skeleton_load(sk, data, size);
	return sk;
}

/* 释放 */
void PI_API pi_skeleton_free(PiSkeleton *sk)
{
	if(sk != NULL)
	{
		pi_skeleton_close(sk);
		pi_free(sk);
	}
}

PiBool PI_API pi_skeleton_load(PiSkeleton *sk, byte *data, uint32 size)
{
	uint32 i;
	Bone *bones;
	PiBytes bb;
	pi_memset_inline(sk, 0, sizeof(PiSkeleton));
	if(!check_sk_header(data, size))
		return FALSE;

	data += SKELETON_HEADER_LEN;
	size -= SKELETON_HEADER_LEN;
	pi_bytes_load(&bb, data, size, FALSE);
	sk_load_impl(sk, &bb);
	pi_bytes_clear(&bb, FALSE);

	bones = sk->boneData.data;
	sk->isCacheCompute = pi_malloc(sk->boneData.elemNum * sizeof(PiBool));
	sk->cacheTrans = pi_malloc(sk->boneData.elemNum * sizeof(TransformData));	
	for(i = 0; i < sk->boneData.elemNum; ++i)
	{
		sk->isCacheCompute[i] = FALSE;
		pi_memcpy_inline(&sk->cacheTrans[i], &bones[i].localData, sizeof(TransformData));
	}
	for(i = 0; i < sk->boneData.elemNum; ++i)
	{
		if(!sk->isCacheCompute[i])
			bone_binding_pose(sk->cacheTrans, bones, sk->isCacheCompute, i);
	}

	sk->invBindPose = pi_malloc(sk->boneData.elemNum * sizeof(TransformData));
	/* 计算初始的binding pose的逆 */
	if(!skeleton_compute_inv_pose(sk->invBindPose, sk->cacheTrans, sk->boneData.elemNum))
	{
		pi_skeleton_close(sk);
		return FALSE;
	}

	return TRUE;
}
static void free_anim_data( BufferData *animData )
{
	uint32 i, j;
	Animation *anims = animData->data;

	/* 释放动画数据，三层 */
	for(i = 0; i < animData->elemNum; ++i)
	{
		Animation *pAnim = anims + i;
		/*释放缓存数据*/
		if (pAnim->matrix_cache.elemNum > 0)
		{
			for (j = 0; j < pAnim->matrix_cache.elemNum; j++)
			{
				if (((float**)pAnim->matrix_cache.data)[j] != NULL)
				{
					pi_free(((float**)pAnim->matrix_cache.data)[j]);
				}
			}
			pi_free(pAnim->matrix_cache.data);
		}

		AnimTrack *tracks = pAnim->tracks.data;
		
		pi_free(pAnim->name);
		for(j = 0; j < pAnim->tracks.elemNum; ++j)
		{
			AnimTrack *pTrack = tracks + j;
			bufferdata_free(&pTrack->keyframes);
		}
		bufferdata_free(&pAnim->tracks);
	}
	bufferdata_free(animData);
}



void PI_API pi_skeleton_close(PiSkeleton *sk)
{
	pi_free(sk->isCacheCompute);
	pi_free(sk->cacheTrans);
	pi_free(sk->invBindPose);

	/* 释放骨头数据 */
	free_bone_data(&sk->boneData);

	free_anim_data(&sk->animData);
}

void PI_API pi_skeleton_merge_anim(PiSkeleton *dstSk, PiSkeleton *srcSk )
{
	uint32 i = 0;
	uint32 j = 0;
	uint32 dstAnimNum = dstSk->animData.elemNum;
	uint32 srcAnimNum = srcSk->animData.elemNum;
	uint32 totalAnimNum = dstAnimNum + srcAnimNum;
	Animation *pNewAnimData = pi_new0(Animation, totalAnimNum);
	Animation *dstAnim = dstSk->animData.data;
	Animation *srcAnim = srcSk->animData.data;

	AnimTrack *pNewTrack;
	AnimTrack *pDstTrack;
	AnimTrack *pSrcTrack;
	
	pi_memcpy_inline( pNewAnimData, dstAnim, sizeof(Animation) * dstAnimNum );
	pi_memcpy_inline( pNewAnimData + dstAnimNum, srcAnim, sizeof(Animation) * srcAnimNum );
	
	
	//拷贝名字
	for( i=0; i < dstAnimNum; ++i )
	{
		pNewAnimData[i].name = pi_new0(wchar, pi_wstrlen(dstAnim[i].name) + 1);
		pi_wstrcpy( pNewAnimData[i].name, dstAnim[i].name, pi_wstrlen(dstAnim[i].name) );
		pNewAnimData[i].tracks.data = pi_new0( AnimTrack, dstAnim[i].tracks.elemNum );
		pi_memcpy_inline(pNewAnimData[i].tracks.data, dstAnim[i].tracks.data, dstAnim[i].tracks.elemSize * dstAnim[i].tracks.elemNum);

		pNewTrack = pNewAnimData[i].tracks.data;
		pDstTrack = dstAnim[i].tracks.data;
		for( j = 0; j < dstAnim[i].tracks.elemNum; ++j )
		{
			pNewTrack->keyframes.data = pi_new0(AnimKeyFrame, pNewTrack->keyframes.elemNum);
			pi_memcpy_inline(pNewTrack->keyframes.data, pDstTrack->keyframes.data, pDstTrack->keyframes.elemNum * pDstTrack->keyframes.elemSize);
			pNewTrack += 1;
			pDstTrack += 1;
		}
	}
	for( ; i < totalAnimNum; ++i )
	{
		pNewAnimData[i].name = pi_new0(wchar, pi_wstrlen(srcAnim[i - dstAnimNum].name) + 1 );
		pi_wstrcpy( pNewAnimData[i].name, srcAnim[i - dstAnimNum].name, pi_wstrlen(srcAnim[i - dstAnimNum].name) );

		pNewAnimData[i].tracks.data = pi_new0( AnimTrack, srcAnim[i - dstAnimNum].tracks.elemNum );
		pi_memcpy_inline(pNewAnimData[i].tracks.data, srcAnim[i - dstAnimNum].tracks.data, srcAnim[i - dstAnimNum].tracks.elemSize * srcAnim[i - dstAnimNum].tracks.elemNum);

		pNewTrack = pNewAnimData[i].tracks.data;
		pSrcTrack = srcAnim[i - dstAnimNum].tracks.data;
		for( j = 0; j < srcAnim[i - dstAnimNum].tracks.elemNum; ++j )
		{
			pNewTrack->keyframes.data = pi_new0(AnimKeyFrame, pNewTrack->keyframes.elemNum);
			pi_memcpy_inline(pNewTrack->keyframes.data, pSrcTrack->keyframes.data, pSrcTrack->keyframes.elemNum * pSrcTrack->keyframes.elemSize);
			pNewTrack += 1;
			pSrcTrack += 1;
		}
	}

	free_anim_data( &dstSk->animData );
	bufferdata_set(&dstSk->animData, sizeof(Animation), totalAnimNum, pNewAnimData);
}

void PI_API pi_skeleton_replace_inv_pose( PiSkeleton *dstSk, PiSkeleton *srcSk )
{
	uint32 numBone = 0;

	if(dstSk->invBindPose == NULL || srcSk->invBindPose == NULL)
		PI_ASSERT(FALSE, "invBindPos is NULL");

	numBone = (dstSk->boneData.elemNum < srcSk->boneData.elemNum) ? dstSk->boneData.elemNum : srcSk->boneData.elemNum;

	pi_memcpy_inline(dstSk->invBindPose, srcSk->invBindPose, sizeof(TransformData) * numBone);
}

static Animation* find_anim_by_index(PiSkeleton *skeleton, uint32 animID)
{
	Animation *anim = skeleton->animData.data;
	if(animID < skeleton->animData.elemNum)
		return &anim[animID];
	return NULL;
}

uint32 PI_API pi_skeleton_get_anim_id( PiSkeleton *sk, wchar *name )
{
	uint32 i;
	Animation *anim = sk->animData.data;
	for(i=0; i < sk->animData.elemNum; ++i )
	{
		if( pi_wstr_equal( anim[i].name, name, TRUE ) )
			return i;
	}
	
	return 0;
}

uint32 PI_API pi_skeleton_get_anim_time( PiSkeleton *sk, uint32 animID )
{
	Animation *anim = find_anim_by_index(sk, animID);
	return ( anim != NULL ) ? anim->playTime : 0;
}


static void keyframe_interpolate(AnimKeyFrame *dst, AnimTrack *pTrack, uint32 keyTime)
{
	uint32 i;
	float frac = 0.0f;		/* 插值百分比 */
	uint32 lastId = 0, nextId = 0;	/* 上一个与下一个帧的ID */
	AnimKeyFrame *pKeyFrame = pTrack->keyframes.data;

	dst->startTime = keyTime;

	/* 比最后一帧还要大，设到最后一帧的时间 */
	if(keyTime > pKeyFrame[pTrack->keyframes.elemNum - 1].startTime)
		keyTime = pKeyFrame[pTrack->keyframes.elemNum - 1].startTime;

	/* 查找开始时间介于keyTime的前后帧 */
	for(i = 0; i < pTrack->keyframes.elemNum; ++i)
	{
		if(keyTime > pKeyFrame[i].startTime)
			lastId = i;
		else if(keyTime == pKeyFrame[i].startTime)
		{/* 等于时候，无需插值，直接跳过 */
			lastId = nextId = i;
			break;
		}
		else 
		{
			nextId = i;
			break;
		}
	}

	if(lastId == nextId)
	{/* 不需要插值，直接拷贝该帧数据到dst */
		pi_vec3_copy(&dst->transform.translate, &pKeyFrame[lastId].transform.translate);
		pi_vec3_copy(&dst->transform.scale, &pKeyFrame[lastId].transform.scale);
		pi_quat_copy(&dst->transform.rotate, &pKeyFrame[lastId].transform.rotate);
		return;
	}

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

static void transform_apply_keyframe(TransformData *dst, TransformData *src, AnimKeyFrame *keyFrame)
{
	pi_vec3_add(&dst->translate, &src->translate, &keyFrame->transform.translate);
	pi_quat_mul(&dst->rotate, &src->rotate, &keyFrame->transform.rotate);
	pi_vec3_mul(&dst->scale, &src->scale, &keyFrame->transform.scale);
}

static void bone_compute_offset_mat(PiMatrix4 *dst, TransformData *invData, TransformData *pData)
{
	PiVector3 locScale;
	PiVector3 locTranslate;
	PiQuaternion locRotate;

	pi_vec3_mul(&locScale, &pData->scale, &invData->scale);
	pi_quat_mul(&locRotate, &pData->rotate, &invData->rotate);

	pi_vec3_mul(&locTranslate, &locScale, &invData->translate);
	pi_quat_rotate_vec3(&locTranslate, &locTranslate, &locRotate);
	pi_vec3_add(&locTranslate, &pData->translate, &locTranslate);
	pi_mat4_build_transform(dst, &locTranslate, &locScale, &locRotate);
}

static void skeleton_init_anim_pose(PiSkeleton *sk, uint32 boneID, PiMatrix4 *dst)
{
	if(!sk->isCacheCompute[boneID])
		bone_binding_pose(sk->cacheTrans, sk->boneData.data, sk->isCacheCompute, boneID);

	bone_compute_offset_mat(dst, &sk->invBindPose[boneID], &sk->cacheTrans[boneID]);
}

static void keyframe_apply_weight(AnimKeyFrame *frame, float weight)
{
	PiVector3 scaleUnit;

	pi_vec3_scale(&frame->transform.translate, &frame->transform.translate, weight);

	pi_quat_lerp(&frame->transform.rotate, pi_quat_get_unit(), &frame->transform.rotate, weight, TRUE);

	pi_vec3_set(&scaleUnit, 1.0f, 1.0f, 1.0f);
	if(!pi_vec3_is_equal(&frame->transform.scale, &scaleUnit))
	{
		pi_vec3_sub(&frame->transform.scale, &frame->transform.scale, &scaleUnit);
		pi_vec3_scale(&frame->transform.scale, &frame->transform.scale, weight);
		pi_vec3_add(&frame->transform.scale, &frame->transform.scale, &scaleUnit);
	}
}

static void skeleton_on_anim(PiSkeleton *sk, Animation *anim, uint32 time, float weight)
{
	uint32 i;
	AnimTrack *tracks;
	
	if(weight <= 0)
		return;

	if(anim->playTime == 0)
		time = 0;
	else
		time %= anim->playTime;

	tracks = anim->tracks.data;
	for(i = 0; i < anim->tracks.elemNum; ++i)
	{
		AnimKeyFrame keyFrame;
		uint32 boneID = tracks[i].boneId;
		keyframe_interpolate(&keyFrame, tracks + i, time);
		if(!IS_FLOAT_EQUAL(weight, 1.0f))
			keyframe_apply_weight(&keyFrame, weight);
		transform_apply_keyframe(&sk->cacheTrans[boneID], &sk->cacheTrans[boneID], &keyFrame);
	}
	
};

float* PI_API pi_skeleton_set_cache_matrix(PiSkeleton *anim, uint frame_index, float* data)
{
	Animation *animData = (Animation*)anim->animData.data;
	uint data_size = anim->boneData.elemNum * 12 * sizeof(float);
	uint index = frame_index % animData->matrix_cache.elemNum;
	animData->matrix_cache.data;
	if (((float**)animData->matrix_cache.data)[index] == NULL)
	{
		((float**)animData->matrix_cache.data)[index] = pi_malloc(data_size);
		pi_memcpy_inline(((float**)animData->matrix_cache.data)[index], data, data_size);
	}
	return ((float**)animData->matrix_cache.data)[index];
}
uint PI_API pi_skanim_get_cache_num(PiSkeleton* anim)
{
	Animation* data = (Animation*)anim->animData.data;
	if (data->matrix_cache.elemNum == 0)
	{
		uint frame_count = (uint)(data->playTime / frame_delta_time);
		data->matrix_cache.elemNum = frame_count + 1;
		data->matrix_cache.elemSize = sizeof(float*);
		data->matrix_cache.data = pi_new0(float*, data->matrix_cache.elemNum);
	}
	return data->matrix_cache.elemNum;
}

float* PI_API pi_skeleton_get_cache_matrix(PiSkeleton *anim, uint frame_index)
{
	Animation *data = (Animation*)anim->animData.data;
	if (data->matrix_cache.elemNum == 0)
	{
		uint frame_count = (uint)(data->playTime / frame_delta_time);
		data->matrix_cache.elemNum = frame_count + 1;
		data->matrix_cache.elemSize = sizeof(float*);
		data->matrix_cache.data = pi_new0(float*, data->matrix_cache.elemNum);
		return NULL;
	}
	return ((float**)data->matrix_cache.data)[frame_index % data->matrix_cache.elemNum];
}

uint PI_API pi_skeleton_get_animation_time(PiSkeleton *anim)
{
	Animation *data = (Animation *)anim->animData.data;
	return (data != NULL) ? data->playTime : 0;
}

void PI_API pi_skeleton_init_cache_data(PiSkeleton* anim, float delta_)
{
}

static void skeleton_get_anim_matrix(PiSkeleton *sk, uint32 time, float weight, PiSkeleton *blendSk, uint32 blendTime)
{
	uint32 i;
	if(sk != NULL)
	{
		Animation *anim = NULL;
		Bone *bones = NULL;
		Bone *blendBones = NULL;
		bones = sk->boneData.data;
		pi_memset_inline(sk->isCacheCompute, FALSE, sizeof(PiBool) * sk->boneData.elemNum);
		
		if(blendSk != NULL)
		{
			blendBones = blendSk->boneData.data;

			for(i = 0; i < sk->boneData.elemNum; ++i)
			{
				pi_vec3_lerp(&sk->cacheTrans[i].scale, &blendBones[i].localData.scale, &bones[i].localData.scale, weight);
				pi_vec3_lerp(&sk->cacheTrans[i].translate, &blendBones[i].localData.translate, &bones[i].localData.translate, weight);
				pi_quat_lerp(&sk->cacheTrans[i].rotate, &blendBones[i].localData.rotate, &bones[i].localData.rotate, weight, TRUE);
			}
		}
		else
		{
			for(i = 0; i < sk->boneData.elemNum; ++i)
				pi_memcpy_inline(&sk->cacheTrans[i], &bones[i].localData, sizeof(TransformData));
		}

		anim = find_anim_by_index(sk, 0);
		if(NULL == anim)
			return;
		skeleton_on_anim(sk, anim, time, weight);

		//处理混合骨骼
		if(blendSk != NULL)
		{
			anim = find_anim_by_index(blendSk, 0);
			if(NULL == anim)
				return;
			skeleton_on_anim(sk, anim, blendTime, 1.0f - weight);
		}
	}	
}

void PI_API pi_skeleton_anim_apply( PiMatrix4 *mat, uint32 numMat, PiSkeleton *sk, uint32 time, float weight,
								PiSkeleton *blendSk, uint32 blendTime)
{
	uint32 i;
	if(sk != NULL)
	{
		PI_ASSERT(numMat == sk->boneData.elemNum, "numMat != boneNum");

		skeleton_get_anim_matrix(sk, time, weight, blendSk, blendTime);
		
		for(i = 0; i < numMat; ++i)
			skeleton_init_anim_pose(sk, i, &mat[i]);
	}	
}

void PI_API pi_skeleton_get_initposemat(PiSkeleton *sk, PiMatrix4 *dst, uint32 boneID)
{
	if(sk != NULL)
	{
		TransformData transData;
		TransformData *pData = &sk->invBindPose[boneID];
		pi_vec3_set(&transData.translate, -pData->translate.x, -pData->translate.y, -pData->translate.z);
		pi_vec3_set(&transData.scale, 1.0f / pData->scale.x, 1.0f / pData->scale.y, 1.0f / pData->scale.z);
		PI_ASSERT(pi_quat_inverse(&transData.rotate, &pData->rotate), "rotate error");					
		pi_mat4_build_transform(dst, &transData.translate, &transData.scale, &transData.rotate);
	}	
}

/* 得到动画中指定ID骨头的矩阵 */
void PI_API pi_skeleton_get_animmat(PiSkeleton *sk, PiMatrix4 *mat, uint32 boneId, PiMatrix4 *mats, uint32 numMat)
{
	PI_ASSERT(numMat > boneId, "bone id overflow");
	pi_skeleton_get_initposemat(sk, mat, boneId);
	pi_mat4_mul(mat, &mats[boneId], mat);
}

/* 取骨骼对应骨头的数目 */
uint32 PI_API pi_skeleton_get_bonenum(PiSkeleton *sk)
{
	uint32 num = 0;
	if(sk != NULL)
		num = sk->boneData.elemNum;
	return num;
}

/* 通过名字获取骨骼的ID */
uint32 PI_API pi_skeleton_get_bone_id( PiSkeleton *sk, wchar *name )
{
	uint32 num = 0;
	uint32 i = 0;
	Bone *bones = NULL;

	if(sk == NULL )
		return 0;


	num = sk->boneData.elemNum;
	bones = sk->boneData.data;

	for( i = 0; i < num; ++i )
	{
		if( pi_wstr_compare( bones[i].name, name ) == 0 )
		{
			return i;
		}
	}

	return 0;
}
