#include <pi_mesh.h>

#define MESH_VERSION_V02 "TENGINE_MESH_V02"
#define MESH_VERSION_V03 "TENGINE_MESH_V03"
#define MESH_HEADER_LEN sizeof(MESH_VERSION_V02) - 1 + sizeof(uint32)

/* 顶点元素的图形语义 */
typedef enum
{
	EVES_POSITION  = 1,		/* 位置，占3个float */
	EVES_NORMAL    = 2,		/* 法线，占3个float */
	EVES_DIFFUSE   = 3,		/* 漫反射颜色，占4个float */
	EVES_TEXCOORD1 = 4,		/* 1维纹理坐标，占1个float */
	EVES_TEXCOORD2 = 5,		/* 2维纹理坐标，占2个float */
	EVES_TEXCOORD3 = 6,		/* 3维纹理坐标，占3个float */
	EVES_BLEND_WEIGHT = 7,  /* 混合权重 占4个float */
	EVES_BLEND_INDICES = 8, /* 混合索引 占4个unsigned short */
	EVES_TANGENT = 9,		/* 切线, 占4个float */
} EVertexUsage;

static int check_mesh_header(byte *data, uint32 size)
{
	uint32 len = 0;
	uint version;
	uint32 flagLen = sizeof(MESH_VERSION_V02) - 1;
	
	if (size < MESH_HEADER_LEN)
	{
		return FALSE;
	}
	if (0 == pi_memcmp_inline(data, MESH_VERSION_V02, flagLen))
	{
		version = 2;
	}
	else if (0 == pi_memcmp_inline(data, MESH_VERSION_V03, flagLen))
	{
		version = 3;
	}
	else{
		return FALSE;
	}
	len = *(uint32 *)(data + flagLen);
	if (len != size)
	{
		return FALSE;
	}
	return version;
}

static void vertex_data_load(PiRenderData *data, PiBytes *bb, int version)
{
	int i = 0;
	int8 num_vbuf = 0;
	sint vertex_count = 0;
	pi_bytes_read_int(bb, &vertex_count);
	pi_bytes_read_int8(bb, &num_vbuf);

	pi_renderdata_set_vertex_num(data, vertex_count);

	for (i = 0; i < num_vbuf; ++i)
	{
		void *p = NULL;
		int8 usage = 0;
		int8 stage = 0;
		int size = 0;
		pi_bytes_read_int8(bb, &usage);
		pi_bytes_read_int8(bb, &stage);

		switch ((EVertexUsage)usage)
		{
		case EVES_POSITION:
			size = sizeof(PiVector3) * vertex_count;
			pi_bytes_read_data(bb, &p, size);
			pi_renderdata_set_vertex(data, vertex_count,
			                         TRUE, EVS_POSITION, 3, EVT_FLOAT, EVU_STATIC_DRAW, p);
			break;
		case EVES_NORMAL:
			size = sizeof(PiVector3) * vertex_count;
			pi_bytes_read_data(bb, &p, size);
			pi_renderdata_set_vertex(data, vertex_count,
			                         TRUE, EVS_NORMAL, 3, EVT_FLOAT, EVU_STATIC_DRAW, p);
			break;
		case EVES_DIFFUSE:
			size = 4 * sizeof(float) * vertex_count;
			pi_bytes_read_data(bb, &p, size);
			pi_renderdata_set_vertex(data, vertex_count,
			                         TRUE, EVS_DIFFUSE, 4, EVT_FLOAT, EVU_STATIC_DRAW, p);
			break;
		case EVES_BLEND_WEIGHT:
			size = 4 * sizeof(float) * vertex_count;
			pi_bytes_read_data(bb, &p, size);
			pi_renderdata_set_vertex(data, vertex_count,
			                         TRUE, EVS_BLEND_WEIGHTS, 4, EVT_FLOAT, EVU_STATIC_DRAW, p);
			break;
		case EVES_BLEND_INDICES:
			if (version > 2)
			{
				uint16 offset, num;
				pi_bytes_read_uint16(bb, &offset);
				pi_bytes_read_uint16(bb, &num);
				data->bone_offset = offset;
				data->bone_num = num;
			}
			size = 4 * sizeof(uint16) * vertex_count;
			pi_bytes_read_data(bb, &p, size);
			pi_renderdata_set_vertex(data, vertex_count,
			                         TRUE, EVS_BLEND_INDICES, 4, EVT_UNSIGNED_SHORT, EVU_STATIC_DRAW, p);
			break;
		case EVES_TEXCOORD2:
			stage += EVS_TEXCOORD_0;
			size = 2 * sizeof(float) * vertex_count;
			pi_bytes_read_data(bb, &p, size);
			pi_renderdata_set_vertex(data, vertex_count,
 			                         TRUE, stage, 2, EVT_FLOAT, EVU_STATIC_DRAW, p);
			break;
		case EVES_TEXCOORD3:
			stage += EVS_TEXCOORD_1;
			size = 2 * sizeof(float) * vertex_count;
			pi_bytes_read_data(bb, &p, size);
			pi_renderdata_set_vertex(data, vertex_count,
				TRUE, stage, 2, EVT_FLOAT, EVU_STATIC_DRAW, p);
			break;
		case EVES_TANGENT:
			size = 4 * sizeof(float) * vertex_count;
			pi_bytes_read_data(bb, &p, size);
			pi_renderdata_set_vertex(data, vertex_count,
			                         TRUE, EVS_TANGENT, 4, EVT_FLOAT, EVU_STATIC_DRAW, p);
			break;
		default:
			break;
		}
	}
}

static void vertex_data_write(PiBytes *bb, PiRenderData *data)
{
	VertexElement el;
	VertexElement *r;
	uint vbuf_index = 0;
	uint num_vbuf = 0;
	pi_bytes_write_int(bb, data->vertex_num);

	vbuf_index = pi_bytes_size(bb);
	pi_bytes_write_int8(bb, (int8)num_vbuf);

	pi_memset_inline(&el, 0, sizeof(VertexElement));
	el.semantic = EVS_POSITION;
	if (pi_dhash_lookup(&data->vertex_map, &el, &r))
	{
		pi_bytes_write_int8(bb, EVES_POSITION);
		pi_bytes_write_int8(bb, 0);
		pi_bytes_write_data(bb, r->data, sizeof(PiVector3) * data->vertex_num);
		num_vbuf += 1;
	}

	el.semantic = EVS_NORMAL;
	if (pi_dhash_lookup(&data->vertex_map, &el, &r))
	{
		pi_bytes_write_int8(bb, EVES_NORMAL);
		pi_bytes_write_int8(bb, 0);
		pi_bytes_write_data(bb, r->data, sizeof(PiVector3) * data->vertex_num);
		num_vbuf += 1;
	}

	el.semantic = EVS_DIFFUSE;
	if (pi_dhash_lookup(&data->vertex_map, &el, &r))
	{
		pi_bytes_write_int8(bb, EVES_DIFFUSE);
		pi_bytes_write_int8(bb, 0);
		pi_bytes_write_data(bb, r->data, 4 * sizeof(float) * data->vertex_num);
		num_vbuf += 1;
	}

	el.semantic = EVS_BLEND_WEIGHTS;
	if (pi_dhash_lookup(&data->vertex_map, &el, &r))
	{
		pi_bytes_write_int8(bb, EVES_BLEND_WEIGHT);
		pi_bytes_write_int8(bb, 0);
		pi_bytes_write_data(bb, r->data, 4 * sizeof(float) * data->vertex_num);
		num_vbuf += 1;
	}

	el.semantic = EVS_BLEND_INDICES;
	if (pi_dhash_lookup(&data->vertex_map, &el, &r))
	{
		pi_bytes_write_int8(bb, EVES_BLEND_INDICES);
		pi_bytes_write_int8(bb, 0);
		pi_bytes_write_uint16(bb, (uint16)data->bone_offset);
		pi_bytes_write_uint16(bb, (uint16)data->bone_num);
		pi_bytes_write_data(bb, r->data, 4 * sizeof(uint16) * data->vertex_num);
		num_vbuf += 1;
	}

	el.semantic = EVS_TEXCOORD_0;
	if (pi_dhash_lookup(&data->vertex_map, &el, &r))
	{
		pi_bytes_write_int8(bb, EVES_TEXCOORD2);
		pi_bytes_write_int8(bb, 0);
		pi_bytes_write_data(bb, r->data, 2 * sizeof(float) * data->vertex_num);
		num_vbuf += 1;
	}

	el.semantic = EVS_TEXCOORD_1;
	if (pi_dhash_lookup(&data->vertex_map, &el, &r))
	{
		pi_bytes_write_int8(bb, EVES_TEXCOORD2);
		pi_bytes_write_int8(bb, 1);
		pi_bytes_write_data(bb, r->data, 2 * sizeof(float) * data->vertex_num);
		num_vbuf += 1;
	}
	el.semantic = EVS_TANGENT;
	if (pi_dhash_lookup(&data->vertex_map, &el, &r))
	{
		pi_bytes_write_int8(bb, EVES_TANGENT);
		pi_bytes_write_int8(bb, 0);
		pi_bytes_write_data(bb, r->data, r->num * sizeof(float) * data->vertex_num);
		num_vbuf += 1;
	}

	// pi_bytes_windex(bb, vbuf_index);
	// pi_bytes_write_int8(bb, (int8)num_vbuf);
	{
		int8 *buf = pi_bytes_array(bb, vbuf_index);
		*buf = (int8)num_vbuf;
	}
}

static void index_data_load(PiRenderData *data, PiBytes *bb)
{
	sint num = 0;
	int8 size = 0;
	char *p = NULL;
	EIndexType type;
	pi_bytes_read_int(bb, &num);
	pi_bytes_read_int8(bb, &size);
	pi_bytes_read_data(bb, &p, num * size);
	type = (size == 2) ? EINDEX_16BIT : EINDEX_32BIT;
	pi_renderdata_set_index(data, TRUE, num, type, EVU_STATIC_DRAW, p);
}

static void index_data_write(PiBytes *bb, PiRenderData *data)
{
	sint num = data->idata.num;
	int8 size = (data->idata.type == EINDEX_16BIT) ? 2 : 4;
	pi_bytes_write_int(bb, num);
	pi_bytes_write_int8(bb, size);
	pi_bytes_write_data(bb, data->idata.data, size * num);
}

static void pi_renderdata_load(PiRenderData *data, PiBytes *bb, int version)
{
	int8 type = 0;

	/*sint num = 0;
	int8 hasBoneAss = 0;
	float *blend_weights = NULL;
	uint16 *blend_indices = NULL;

	/读取每个顶点的骨头绑定数据
	pi_bytes_read_int8(bb, &hasBoneAss);

	if(hasBoneAss != 0)
	{
		sint i;
		uint16 *curr_index;
		float *curr_weight;
		pi_bytes_read_int(bb, &num);
		curr_index = blend_indices = pi_new0(uint16, num * MAX_BONEINFO_NUM);
		curr_weight = blend_weights = pi_new0(float, num * MAX_BONEINFO_NUM);
		for(i = 0; i <  num; ++i)
		{
			int j = 0;
			int8 bind_num = 0;
			pi_bytes_read_int8(bb, &bind_num);
			PI_ASSERT(bind_num <= MAX_BONEINFO_NUM, "invalid bone info's num");
			for(j = 0; j < bind_num; ++j)
			{
				pi_bytes_read_int16(bb, (int16 *)&curr_index[j]);
				pi_bytes_read_float(bb, &curr_weight[j]);
			}
			curr_index += 4;
			curr_weight += 4;
		}
	}*/

	pi_bytes_read_int8(bb, &type);

	pi_renderdata_init(data, (EGeometryType)type);

	/* 加载索引数据 */
	index_data_load(data, bb);

	/* 加载顶点数据 */
	vertex_data_load(data, bb, version);

	/*if( blend_indices != NULL )
	{
		pi_renderdata_set_vertex(data, data->vertex_num,
			TRUE, EVS_BLEND_INDICES, 4, EVT_SHORT, EVU_STATIC_DRAW, blend_indices);
	}

	if( blend_weights != NULL )
	{
		pi_renderdata_set_vertex(data, data->vertex_num,
			TRUE, EVS_BLEND_WEIGHTS, 4, EVT_FLOAT, EVU_STATIC_DRAW, blend_weights);
	}

	pi_free(blend_indices);
	pi_free(blend_weights);*/
}

static void pi_renderdata_write(PiBytes *bb, PiRenderData *data)
{
	pi_bytes_write_int8(bb, data->type);

	index_data_write(bb, data);

	vertex_data_write(bb, data);
}

static void sub_mesh_norm_weight(uint num, float *weight)
{
	uint32 i;
	for (i = 0; i < num; ++i)
	{
		int8 j;
		float total = 0.0f;
		for (j = 0; j < MAX_BONEINFO_NUM; ++j)
		{
			total += weight[j];
		}

		if (!IS_FLOAT_EQUAL(total, 0.05f))
		{
			for (j = 0; j < MAX_BONEINFO_NUM; ++j)
			{
				weight[j] /= total;
				//weight[j] = 1.0f;
			}
		}
		else
		{
			weight[0] = 1.0f;
		}

		weight += MAX_BONEINFO_NUM;
	}
}

void PI_API pi_mesh_copy( PiMesh *dst, PiMesh *src )
{
	pi_renderdata_set(&dst->data, &src->data, TRUE);
}

void PI_API pi_mesh_interpolate(PiMesh *dst_mesh, PiMesh *start_mesh, PiMesh *end_mesh, float t)
{
	uint32 vertex_num = dst_mesh->data.vertex_num;
	VertexElement *start_elem, *end_elem, *dst_elem;
	float *start_fdata, *end_fdata, *dst_fdata;

	uint32 i;

	PI_ASSERT(vertex_num == start_mesh->data.vertex_num, "mesh interpolate error, vertex num does not match");
	PI_ASSERT(vertex_num == end_mesh->data.vertex_num, "mesh interpolate error, vertex num does not match");

	//位置插值
	start_elem = pi_renderdata_get_vertex(&start_mesh->data, EVS_POSITION);
	end_elem = pi_renderdata_get_vertex(&end_mesh->data, EVS_POSITION);
	dst_elem = pi_renderdata_get_vertex(&dst_mesh->data, EVS_POSITION);

	start_fdata = (float *)start_elem->data;
	end_fdata = (float *)end_elem->data;
	dst_fdata = (float *)dst_elem->data;

	for (i = 0; i < vertex_num * start_elem->num; ++i )
	{
		dst_fdata[i] = start_fdata[i] * (1.0f - t) + end_fdata[i] * t;
	}

	//第一层纹理坐标
	start_elem = pi_renderdata_get_vertex(&start_mesh->data, EVS_TEXCOORD_0);
	end_elem = pi_renderdata_get_vertex(&end_mesh->data, EVS_TEXCOORD_0);
	dst_elem = pi_renderdata_get_vertex(&dst_mesh->data, EVS_TEXCOORD_0);
	if ( start_elem != NULL && end_elem != NULL && dst_elem != NULL)
	{
		start_fdata = (float *)start_elem->data;
		end_fdata = (float *)end_elem->data;
		dst_fdata = (float *)dst_elem->data;

		for (i = 0; i < vertex_num * start_elem->num; ++i )
		{
			dst_fdata[i] = start_fdata[i] * (1.0f - t) + end_fdata[i] * t;
		}
	}

	//法线
	start_elem = pi_renderdata_get_vertex(&start_mesh->data, EVS_NORMAL);
	end_elem = pi_renderdata_get_vertex(&end_mesh->data, EVS_NORMAL);
	dst_elem = pi_renderdata_get_vertex(&dst_mesh->data, EVS_NORMAL);
	if ( start_elem != NULL && end_elem != NULL && dst_elem != NULL)
	{
		start_fdata = (float *)start_elem->data;
		end_fdata = (float *)end_elem->data;
		dst_fdata = (float *)dst_elem->data;
		for (i = 0; i < vertex_num * start_elem->num; ++i )
		{
			dst_fdata[i] = start_fdata[i] * (1.0f - t) + end_fdata[i] * t;
		}
	}

	//颜色
	start_elem = pi_renderdata_get_vertex(&start_mesh->data, EVS_DIFFUSE);
	end_elem = pi_renderdata_get_vertex(&end_mesh->data, EVS_DIFFUSE);
	dst_elem = pi_renderdata_get_vertex(&dst_mesh->data, EVS_DIFFUSE);
	if ( start_elem != NULL && end_elem != NULL && dst_elem != NULL)
	{
		float *start_color = (float *)start_elem->data;
		float *end_color = (float *)end_elem->data;
		float *dst_color = (float *)dst_elem->data;

		for (i = 0; i < vertex_num * start_elem->num; ++i )
		{
			dst_color[i] = start_color[i] * (1.0f - t) + end_color[i] * t;
		}
	}
}

PiMesh *PI_API pi_mesh_gen_integral_mesh( PiMesh **src, uint32 num)
{
	uint32 i, j;
	EIndexType itype = EINDEX_16BIT;
	EGeometryType gtype = EGOT_POINT_LIST;

	PiVector3 *pos = NULL;
	PiVector3 *normal = NULL;
	float *color = NULL;
	float *texcoord = NULL;
	uint16 *blendIndices = NULL;
	float *blendWeights = NULL;
	PiBool hasBoneInfo = FALSE;
	PiBool hasColorInfo = FALSE;

	uint32 indexNum = 0;
	uint32 vertexNum = 0;
	uint32 indexCounted = 0;
	uint32 vertexCounted = 0;

	void *indexData = NULL;
	PiMesh *mesh = pi_new0(PiMesh, 1);

	/* 做一些可能的错误判断, 计算总共的索引数量和顶点数量 */
	for (i = 0; i < num; ++i)
	{
		if ( i == 0 )
		{
			gtype = src[i]->data.type;
			itype = src[i]->data.idata.type;
		}
		else if ( gtype != src[i]->data.type )
		{
			PI_ASSERT(FALSE, "submesh geometry type differs, all the submeshes' geometry type should be the same" );
		}
		else if ( itype != src[i]->data.idata.type )
		{
			PI_ASSERT(FALSE, "submesh index type differs, all the submeshes' index type should be the same" );
		}

		indexNum += src[i]->data.idata.num;
		vertexNum += src[i]->data.vertex_num;
	}

	// 拷贝索引数据
	if ( itype == EINDEX_16BIT )
	{
		uint16 *indexData16;
		uint16 *srcIndexData16;
		indexData = pi_new0(uint16, indexNum);
		indexData16 = indexData;
		for ( i = 0; i < num; ++i )
		{
			srcIndexData16 = (uint16 *)src[i]->data.idata.data;

			/* 更改索引 */
			for ( j = 0; j < src[i]->data.idata.num; ++j )
			{
				indexData16[j + indexCounted] = (uint16)(srcIndexData16[j] + vertexCounted);
			}

			indexCounted += src[i]->data.idata.num;
			vertexCounted += src[i]->data.vertex_num;
		}
	}
	else
	{
		uint32 *indexData32;
		uint32 *srcIndexData32;
		indexData = pi_new0(uint32, indexNum);
		indexData32 = indexData;
		for ( i = 0; i < num; ++i )
		{
			srcIndexData32 = (uint32 *)src[i]->data.idata.data;

			for ( j = 0; j < src[i]->data.idata.num; ++j )
			{
				indexData32[j + indexCounted] = srcIndexData32[j] + vertexCounted;
			}

			vertexCounted += src[i]->data.vertex_num;
			indexCounted += src[i]->data.idata.num;
		}
	}

	// 拷贝顶点数据
	pos = pi_new0(PiVector3, vertexNum);
	normal = pi_new0(PiVector3, vertexNum);
	texcoord = pi_new0(float, 2 * vertexNum);
	color = pi_new0(float, 4 * vertexNum);
	blendIndices = pi_new0(uint16, vertexNum * MAX_BONEINFO_NUM);
	blendWeights = pi_new0(float, vertexNum * MAX_BONEINFO_NUM);

	vertexCounted = 0;
	for ( i = 0; i < num; ++i )
	{
		VertexElement *elem = pi_renderdata_get_vertex(&src[i]->data, EVS_POSITION);
		if (elem != NULL)
		{
			pi_memcpy_inline(pos + vertexCounted, elem->data, elem->size);
		}

		elem = pi_renderdata_get_vertex(&src[i]->data, EVS_NORMAL);
		if (elem != NULL)
		{
			pi_memcpy_inline(normal + vertexCounted, elem->data, elem->size);
		}

		elem = pi_renderdata_get_vertex(&src[i]->data, EVS_DIFFUSE);
		if (elem != NULL)
		{
			hasColorInfo = TRUE;
			pi_memcpy_inline(color + 4 * vertexCounted, elem->data, elem->size);
		}

		/* 只考虑第一层texcoord */
		elem = pi_renderdata_get_vertex(&src[i]->data, EVS_TEXCOORD_0);
		if (elem != NULL)
		{
			pi_memcpy_inline(texcoord + 2 * vertexCounted, elem->data, elem->size);
		}

		elem = pi_renderdata_get_vertex(&src[i]->data, EVS_BLEND_INDICES);
		if (elem != NULL)
		{
			hasBoneInfo = TRUE;
			pi_memcpy_inline(blendIndices + vertexCounted * MAX_BONEINFO_NUM, elem->data, elem->size);
		}

		elem = pi_renderdata_get_vertex(&src[i]->data, EVS_BLEND_WEIGHTS);
		if (elem != NULL)
		{
			hasBoneInfo = TRUE;
			pi_memcpy_inline(blendWeights + vertexCounted * MAX_BONEINFO_NUM, elem->data, elem->size);
		}

		vertexCounted += src[i]->data.vertex_num;
	}

	if ( hasColorInfo )
	{
		pi_free(color);
		color = NULL;
	}

	if (!hasBoneInfo)
	{
		pi_free( blendIndices );
		blendIndices = NULL;
		pi_free( blendWeights );
		blendWeights = NULL;
	}

	// 根据组合出来的整块数据, 创建新模型的渲染数据
	mesh = pi_mesh_create(gtype, itype,
	                      vertexNum, pos, color, normal, texcoord, indexNum, indexData);

	if (blendIndices != NULL)
	{
		pi_renderdata_set_vertex(&mesh->data, vertexNum,
		                         TRUE, EVS_BLEND_INDICES, 4, EVT_SHORT, EVU_STATIC_DRAW, blendIndices);
	}
	if (blendWeights != NULL)
	{
		pi_renderdata_set_vertex(&mesh->data, vertexNum,
		                         TRUE, EVS_BLEND_WEIGHTS, 4, EVT_FLOAT, EVU_STATIC_DRAW, blendWeights);
	}

	pi_free( pos );
	pi_free( normal );
	pi_free( texcoord );
	pi_free( indexData );

	if ( color )
	{
		pi_free( color );
	}

	if (blendIndices != NULL)
	{
		pi_free( blendIndices );
	}
	if (blendWeights != NULL)
	{
		pi_free( blendWeights );
	}

	return mesh;
}

PiMesh *PI_API pi_mesh_create(EGeometryType gtype,
                              EIndexType itype, uint32 vertex_num, void *pos, void *color,
                              void *normal, void *texcoord, uint32 index_num, void *index)
{
	int index_size = 0;
	PiMesh *mesh = pi_new0(PiMesh, 1);
	PiRenderData *data = &mesh->data;

	pi_renderdata_init(data, gtype);

	index_size = (itype == EINDEX_16BIT) ? 2 : 4;
	pi_renderdata_set_index(data, TRUE, index_num, itype, EVU_STATIC_DRAW, index);

	pi_renderdata_set_vertex_num(data, vertex_num);

	if (pos != NULL)
	{
		pi_renderdata_set_vertex(data, vertex_num,
		                         TRUE, EVS_POSITION, 3, EVT_FLOAT, EVU_STATIC_DRAW, pos);
	}

	if (color != NULL)
	{
		pi_renderdata_set_vertex(data, vertex_num,
		                         TRUE, EVS_DIFFUSE, 4, EVT_FLOAT, EVU_STATIC_DRAW, color);
	}

	if (normal != NULL)
	{
		pi_renderdata_set_vertex(data, vertex_num,
		                         TRUE, EVS_NORMAL, 3, EVT_FLOAT, EVU_STATIC_DRAW, normal);
	}

	if (texcoord != NULL)
	{
		pi_renderdata_set_vertex(data, vertex_num,
		                         TRUE, EVS_TEXCOORD_0, 2, EVT_FLOAT, EVU_STATIC_DRAW, texcoord);
	}

	return mesh;
}

PiMesh *PI_API pi_mesh_create_empty(void)
{
	return pi_new0(PiMesh, 1);
}

/* 释放网格 */
void PI_API pi_mesh_free(PiMesh *mesh)
{
	if (mesh != NULL)
	{
		pi_renderdata_clear(&mesh->data);
		pi_free(mesh);
	}
}

uint32 PI_API pi_mesh_num(byte *data, uint32 size)
{
	PiBytes bb;
	uint32 num = 0;
	int version;
	version = check_mesh_header(data, size);
	if (version > 0)
	{
		data += MESH_HEADER_LEN;
		size -= MESH_HEADER_LEN;
		pi_bytes_load(&bb, data, size, FALSE);
		pi_bytes_read_int(&bb, (sint *)&num);
		pi_bytes_clear(&bb, FALSE);
	}
	return num;
}

PiBool PI_API pi_mesh_load(PiMesh **meshs, uint32 num, byte *data, uint32 size)
{
	PiBytes bb;
	PiBool r = FALSE;
	uint32 i, num_impl = 0;
	int version = check_mesh_header(data, size);
	if (version > 0)
	{
		r = TRUE;
		data += MESH_HEADER_LEN;
		size -= MESH_HEADER_LEN;
		pi_bytes_load(&bb, data, size, FALSE);
		pi_bytes_read_int(&bb, (sint *)&num_impl);
		PI_ASSERT(num_impl == num, "submesh's num isn't same");

		for (i = 0; i < num; ++i)
		{
			pi_memset_inline(meshs[i], 0, sizeof(PiMesh));
			meshs[i]->version = version;
			pi_renderdata_load(&meshs[i]->data, &bb, version);
		}

		pi_bytes_clear(&bb, FALSE);
	}
	return r;
}

/* 将网格数据反序列化成二进制数据 */
void PI_API pi_mesh_write(PiBytes *dst, PiMesh **meshes, uint32 num)
{
	void *data;
	uint32 i, size, vLen = sizeof(MESH_VERSION_V03) - 1;
	pi_bytes_write_data(dst, MESH_VERSION_V03, vLen);
	pi_bytes_write_int32(dst, 0);
	pi_bytes_write_int32(dst, num);
	for (i = 0; i < num; ++i)
	{
		pi_renderdata_write(dst, &meshes[i]->data);
	}

	size = pi_bytes_size(dst);
	data = pi_bytes_array(dst, vLen);
	pi_memcpy_inline(data, &size, sizeof(uint32));
}

/* 将骨骼矩阵作用于网格位置和法线 */
static void pos_normal_apply_skmat(PiRenderData *data, PiAABBBox *box, PiMatrix4 *skmat, uint32 num_skmat, PiRenderData *srcData)
{
	uint32 i;
	PiVector3 *pos = NULL;
	PiVector3 *normal = NULL;
	uint16 *index = NULL;
	float *weight = NULL;
	VertexElement *normal_vertex = pi_renderdata_get_vertex(data, EVS_NORMAL);
	VertexElement *pos_vertex = pi_renderdata_get_vertex(data, EVS_POSITION);
	VertexElement *index_vertex = pi_renderdata_get_vertex(srcData, EVS_BLEND_INDICES);
	VertexElement *weight_vertex = pi_renderdata_get_vertex(srcData, EVS_BLEND_WEIGHTS);

	if (pos_vertex != NULL)
	{
		pos = pos_vertex->data;
	}
	if (normal_vertex != NULL)
	{
		normal = normal_vertex->data;
	}
	if (index_vertex != NULL)
	{
		index = index_vertex->data;
	}
	if (weight_vertex != NULL)
	{
		weight = weight_vertex->data;
	}

	if (pos != NULL && index != NULL && weight != NULL)
	{
		for (i = 0; i < data->vertex_num; ++i)
		{
			int8 j;
			PiMatrix4 mat;
			pi_memset_inline(&mat, 0, sizeof(PiMatrix4));
			for (j = 0; j < MAX_BONEINFO_NUM; ++j)
			{
				PiMatrix4 temp;
				PI_ASSERT(num_skmat > index[j], "bone id overflow");
				pi_mat4_scale(&temp, skmat + index[j], weight[j]);
				pi_mat4_add(&mat, &mat, &temp);
			}

			pi_mat4_apply_point(&pos[i], &pos[i], &mat);
			if (box)
			{
				pi_aabb_add_point(box, &pos[i]);
			}

			if (normal != NULL)
			{
				/* 是mat的转置矩阵的逆 */
				PiMatrix4 normal_mat;
				pi_mat4_copy(&normal_mat, &mat);
				pi_mat4_transpose(&normal_mat, &normal_mat);
				pi_mat4_inverse(&normal_mat, &normal_mat);
				pi_mat4_apply_vector(&normal[i], &normal[i], &normal_mat);
				if (!IS_FLOAT_EQUAL(1.0f, pi_vec3_len_square(&normal[i])))
				{
					pi_vec3_normalise(&normal[i], &normal[i]);
				}
			}

			index += 4;
			weight += 4;
		}
	}
}

/* 应用骨骼矩阵和世界矩阵计算骨骼数据，同时更新box */
PiRenderData *PI_API pi_mesh_apply(PiMesh *mesh, PiBool is_normal, PiAABBBox *box, uint32 num_skmat, PiMatrix4 *skmat)
{
	PiRenderData *data = pi_new0(PiRenderData, 1);
	pi_renderdata_init(data, EGOT_TRIANGLE_LIST);
	data->vertex_num = mesh->data.vertex_num;
	if (NULL != skmat)
	{
		/* 深拷贝其中的位置和法线数据 */
		VertexElement *elem = pi_renderdata_get_vertex(&mesh->data, EVS_POSITION);
		pi_renderdata_set_vertex(data, mesh->data.vertex_num,
		                         TRUE, EVS_POSITION, 3, EVT_FLOAT, EVU_STATIC_DRAW, elem->data);

		if (!is_normal)
		{
			pi_renderdata_set_vertex(data, data->vertex_num,
				TRUE, EVS_NORMAL, 3, EVT_FLOAT, EVU_STATIC_DRAW, NULL);
		}
		else
		{
			VertexElement *r = pi_renderdata_get_vertex(&mesh->data, EVS_NORMAL);
			if (r != NULL)
			{
				pi_renderdata_set_vertex(data, data->vertex_num,
				                         TRUE, EVS_NORMAL, 3, EVT_FLOAT, EVU_STATIC_DRAW, r->data);
			}
		}
		pos_normal_apply_skmat(data, box, skmat, num_skmat, &mesh->data);
	}
	return data;
}

/**
 * 计算网格被骨骼作用之后的AABB，放入：aabb中
 * aabb至少6个元素，分别是：xmin, ymin, zmin, xmax, ymax, zmax
 */
void PI_API pi_mesh_compute_aabb(PiMesh *mesh,
                                 PiAABBBox *aabb, uint32 numSkMat, void *skMat, void *worldMat)
{
	if (mesh != NULL)
	{
		PiAABBBox box;
		if (numSkMat <= 0 || skMat == NULL)
		{
			pi_aabb_copy(&box, &mesh->data.box);
		}
		else
		{
			/* todo 暂用原始aabb,顶点的骨骼矩阵变换太消耗 */
			pi_aabb_copy(&box, &mesh->data.box);
		}

		pi_aabb_transform(&box, worldMat);

		pi_vec3_copy(&aabb->minPt, &box.minPt);
		pi_vec3_copy(&aabb->maxPt, &box.maxPt);
	}
}

/**
 * 取得网格的顶点数据并返回
 * gtype: 返回顶点的几何类型
 * vertex_num：返回顶点的数量
 * 函数返回值：顶点的位置数据
 */
PiVector3 *PI_API pi_mesh_get_pos(PiMesh *mesh, EGeometryType *gtype, uint32 *vertex_num)
{
	PiVector3 *pos = NULL;
	if (mesh != NULL)
	{
		VertexElement *r = pi_renderdata_get_vertex(&mesh->data, EVS_POSITION);
		if (gtype != NULL)
		{
			*gtype = mesh->data.type;
		}
		if (vertex_num != NULL)
		{
			*vertex_num = mesh->data.vertex_num;
		}
		if (r != NULL)
		{
			pos = r->data;
		}
	}
	return pos;
}

int32 PI_API pi_mesh_get_texcoord_count(PiMesh *mesh)
{
	int32 num = 0;
	if (mesh != NULL)
	{
		int32 i;
		for (i = 0; i < MAX_TEXCOORD_NUM; ++i)
		{
			VertexElement *r = pi_renderdata_get_vertex(&mesh->data, i + EVS_TEXCOORD_0);
			if (r != NULL)
			{
				++num;
			}
		}
	}
	return num;
}

float *PI_API pi_mesh_get_texcoord(PiMesh *mesh, uint layer, uint32 *texcoord_num, uint *size)
{
	float *texcoord = NULL;
	if (mesh != NULL)
	{
		VertexElement *r = pi_renderdata_get_vertex(&mesh->data, (VertexSemantic)(EVS_TEXCOORD_0 + layer));

		if (texcoord_num != NULL)
		{
			*texcoord_num = mesh->data.vertex_num;
		}

		if (size != NULL)
		{
			*size = r->num;
		}

		if (r != NULL)
		{
			texcoord = (float *)r->data;
		}
	}

	return texcoord;
}

PiVector3 *pi_mesh_get_normal(PiMesh *mesh, uint32 *vertex_num)
{
	PiVector3 *normal = NULL;
	if (mesh != NULL)
	{
		VertexElement *r = pi_renderdata_get_vertex(&mesh->data, EVS_NORMAL);

		if (vertex_num != NULL)
		{
			*vertex_num = mesh->data.vertex_num;
		}

		if (r != NULL)
		{
			normal = r->data;
		}
	}

	return normal;
}

void *PI_API pi_mesh_get_index(PiMesh *mesh, EIndexType *itype, uint32 *index_num)
{
	void *index = NULL;
	if (mesh != NULL)
	{
		PiRenderData *data = &mesh->data;
		if (itype != NULL)
		{
			*itype = data->idata.type;
		}
		if (index_num != NULL)
		{
			*index_num = data->idata.num;
		}
		index = data->idata.data;
	}
	return index;
}


uint PI_API pi_mesh_get_vertex_num(PiMesh *mesh)
{
	uint num = 0;
	if (mesh != NULL)
	{
		PiRenderData *data = &mesh->data;
		num = data->vertex_num;
	}
	return num;
}

uint PI_API pi_mesh_get_face_num(PiMesh *mesh)
{
	uint num = 0;
	EGeometryType type = mesh->data.type;

	switch (type)
	{
	case EGOT_POINT_LIST:
		num =  mesh->data.vertex_num;
		break;
	case EGOT_LINE_LIST:
		if (mesh->data.idata.num > 0)
		{
			num =  mesh->data.idata.num / 2;
		}
		else
		{
			num =  mesh->data.vertex_num / 2;
		}

		break;
	case EGOT_LINE_STRIP:
		if (mesh->data.idata.num > 0)
		{
			num =  mesh->data.idata.num - 1;
		}
		else
		{
			num =  mesh->data.vertex_num - 1;
		}
		break;
	case EGOT_TRIANGLE_LIST:
		if (mesh->data.idata.num > 0)
		{
			num =  mesh->data.idata.num / 3;
		}
		else
		{
			num =  mesh->data.vertex_num / 3;
		}
		break;
	case EGOT_TRIANGLE_STRIP:
		if (mesh->data.idata.num > 0)
		{
			num =  mesh->data.idata.num / 2;
		}
		else
		{
			num =  (mesh->data.vertex_num / 2);
		}
		break;
	case EGOT_TRIANGLE_FAN:
		if (mesh->data.idata.num > 0)
		{
			num =  (mesh->data.idata.num - 2 );
		}
		else
		{
			num =  (mesh->data.vertex_num - 2 );
		}
		break;
	case EGOT_QUAD_LIST:
		if (mesh->data.idata.num > 0)
		{
			num =  mesh->data.idata.num / 4;
		}
		else
		{
			num =  mesh->data.vertex_num / 4;
		}
		break;
	default:
		break;
	}
	return num;
}

PiAABBBox *PI_API pi_mesh_get_aabb(PiMesh *mesh)
{
	return &mesh->data.box;
}

PiMesh *PI_API pi_mesh_create_quad(float color[16], float uv[8], float z)
{
	uint32 i;
	PiVector3 normal[4];
	float tcoord[4 * 2] =
	{
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f
	};

	float pos[4 * 3] =
	{
		-0.5f, -0.5f, z,
		0.5f, -0.5f, z,
		0.5f, 0.5f, z,
		-0.5f, 0.5f, z
	};

	uint32 index[6] =
	{
		0, 1, 2,
		0, 2, 3
	};

	for (i = 0; i < 4; ++i)
	{
		pi_vec3_set(&normal[i], 0.0f, 0.0f, 1.0f);
	}

	if (uv == NULL)
	{
		uv = tcoord;
	}

	return pi_mesh_create(EGOT_TRIANGLE_LIST, EINDEX_32BIT, 4, pos, color, normal, uv, 6, index);
}

PiMesh *PI_API pi_mesh_create_plane(float color[16], float uv[8], float y)
{
	uint32 i;
	PiVector3 normal[4];
	float tcoord[4 * 2] =
	{
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};

	float pos[4 * 3] =
	{
		-0.5f, y, 0.5f,
		0.5f, y, 0.5f,
		0.5f, y, -0.5f,
		-0.5f, y, -0.5f
	};

	uint32 index[6] =
	{
		0, 1, 2,
		0, 2, 3
	};

	for (i = 0; i < 4; ++i)
	{
		pi_vec3_set(&normal[i], 0.0f, 1.0f, 0.0f);
	}

	if (uv == NULL)
	{
		uv = tcoord;
	}

	return pi_mesh_create(EGOT_TRIANGLE_LIST, EINDEX_32BIT, 4, pos, color, normal, uv, 6, index);
}

PiMesh* PI_API pi_mesh_create_cylinder(float r, float h)
{
	int numSegs = 20;
	float dphi = PI * 2 / numSegs;
	uint numVertex = 4 * numSegs + 2;
	PiVector3* posBuffer = pi_new(PiVector3, numVertex);
	PiVector3* normalBuffer = pi_new(PiVector3, numVertex);
	PiMesh* mesh;
	uint32* indexBuffer = pi_new(uint32, 4 * numSegs * 3);
	float* texBuffer = pi_new(float, 2 * numVertex);
	posBuffer[numVertex - 2].x = 0.0f;
	posBuffer[numVertex - 2].y = 0.0f;
	posBuffer[numVertex - 2].z = -0.5f * h;

	for (int i = 0; i < numSegs; i++)
	{
		posBuffer[i * 4].x = r * pi_math_cos(i * dphi);
		posBuffer[i * 4].y = r * pi_math_sin(i * dphi);
		posBuffer[i * 4].z = -0.5f * h;
		normalBuffer[i * 4].x = 0.0f;
		normalBuffer[i * 4].y = 0.0f;
		normalBuffer[i * 4].z = -1.0f;
		texBuffer[i * 4 * 2] = posBuffer[i * 4].x;
		texBuffer[i * 4 * 2 + 1] = posBuffer[i * 4].y;


		indexBuffer[i * 4 * 3] = (uint32)(i * 4);
		indexBuffer[i * 4 * 3 + 1] = (uint32)((i * 4 + 4) % (numVertex - 2));
		indexBuffer[i * 4 * 3 + 2] = (uint32)(numVertex - 2);

		texBuffer[i * 4 * 2 + 2] = 1.0f / numSegs;
		texBuffer[i * 4 * 2 + 3] = 0.0f;

		indexBuffer[i * 4 * 3 + 3] = (uint32)((i * 4 + 1) % (numVertex - 2));
		indexBuffer[i * 4 * 3 + 4] = (uint32)((i * 4 + 2) % (numVertex - 2));
		indexBuffer[i * 4 * 3 + 5] = (uint32)((i * 4 + 6) % (numVertex - 2));

		


		indexBuffer[i * 4 * 3 + 6] = (uint32)((i * 4 + 1) % (numVertex - 2));
		indexBuffer[i * 4 * 3 + 7] = (uint32)((i * 4 + 6) % (numVertex - 2));
		indexBuffer[i * 4 * 3 + 8] = (uint32)((i * 4 + 5) % (numVertex - 2));

		indexBuffer[i * 4 * 3 + 9] = (uint32)((i * 4 + 3) % (numVertex - 2));
		indexBuffer[i * 4 * 3 + 10] = (uint32)(numVertex - 1);
		indexBuffer[i * 4 * 3 + 11] = (uint32)((i * 4 + 7) % (numVertex - 2));

		posBuffer[i * 4 + 1].x = r * pi_math_cos(i * dphi);
		posBuffer[i * 4 + 1].y = r * pi_math_sin(i * dphi);
		posBuffer[i * 4 + 1].z = -0.5f * h;
		texBuffer[i * 4 * 2 + 4] = 1.0f / numSegs;
		texBuffer[i * 4 * 2 + 5] = 1;

		normalBuffer[i * 4 + 1].x = pi_math_cos(i * dphi);
		normalBuffer[i * 4 + 1].y = pi_math_sin(i * dphi);
		normalBuffer[i * 4 + 1].z = 0.0f;


		posBuffer[i * 4 + 2].x = r * pi_math_cos(i * dphi);
		posBuffer[i * 4 + 2].y = r * pi_math_sin(i * dphi);
		posBuffer[i * 4 + 2].z = 0.5f * h;

		texBuffer[i * 4 * 2 + 6] = pi_math_cos(i * dphi);
		texBuffer[i * 4 * 2 + 7] = pi_math_sin(i * dphi);


		normalBuffer[i * 4 + 2].x = pi_math_cos(i * dphi);
		normalBuffer[i * 4 + 2].y = pi_math_sin(i * dphi);
		normalBuffer[i * 4 + 2].z = 0.0f;

		posBuffer[i * 4 + 3].x = r * pi_math_cos(i * dphi);
		posBuffer[i * 4 + 3].y = r * pi_math_sin(i * dphi);
		posBuffer[i * 4 + 3].z = 0.5f * h;

		normalBuffer[i * 2 + 3].x = 0.0f;
		normalBuffer[i * 2 + 3].y = 0.0f;
		normalBuffer[i * 2 + 3].z = 1.0f;
	}

	posBuffer[numVertex - 1].x = 0;
	posBuffer[numVertex - 1].y = 0;
	posBuffer[numVertex - 1].z = 0.5f * h;

	texBuffer[numVertex * 2 -4] = 0.5f;
	texBuffer[numVertex * 2 - 3] = 0.5f;
	texBuffer[numVertex * 2 - 2] = 0.5f;
	texBuffer[numVertex * 2 - 1] = 0.5f;
	 
	mesh = pi_mesh_create(EGOT_TRIANGLE_LIST, EINDEX_32BIT, numVertex, posBuffer, NULL, normalBuffer, NULL, 4 * numSegs * 3, indexBuffer);
	pi_free(posBuffer);
	pi_free(normalBuffer);
	pi_free(texBuffer);
	pi_free(indexBuffer);
	return mesh;

}

PiMesh* PI_API pi_mesh_create_capsule(float r, float h)
{
	int n = 10;
	int rows = n * 2 + 2;
	int cels = n * 4 + 1;
	int i, j;
	float* tcoord = pi_new(float, rows * cels * 2);
	float* pos = pi_new(float, rows * cels * 3);
	float* normal = pi_new(float, rows * cels * 3);
	float tcdx = 1.0f / (rows - 1);
	float tcdy = 1.0f / (cels - 1);
	float dgy = PI / (rows - 2);
	float dgx = 2 * PI / (cels - 1);
	PiMesh* mesh;
	uint32* index = pi_new(uint32, (rows - 1) * (cels - 1) * 2 * 3);

	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cels; j++)
		{
			tcoord[(i * cels + j) * 2] = i * tcdx;
			tcoord[(i * cels + j) * 2 + 1] = j * tcdy;

			

			if (i <= n)
			{
				pos[(i * cels + j) * 3] = pi_math_sin((n - i)* dgy) * r + h / 2;
				normal[(i * cels + j) * 3] = pos[(i * cels + j) * 3] - h / 2;



				pos[(i * cels + j) * 3 + 1] = pi_math_cos((n - i) * dgy) * pi_math_sin(j * dgx) * r;
				normal[(i * cels + j) * 3 + 1] = pos[(i * cels + j) * 3 + 1];

				pos[(i * cels + j) * 3 + 2] = pi_math_cos((n - i) * dgy) * pi_math_cos(j * dgx) * r;
				normal[(i * cels + j) * 3 + 2] = pos[(i * cels + j) * 3 + 2];

			}
			else
			{
				pos[(i * cels + j) * 3] = pi_math_sin((n - i - 1)* dgy) * r - h / 2;
				normal[(i * cels + j) * 3] = pos[(i * cels + j) * 3] + h / 2;


				pos[(i * cels + j) * 3 + 1] = pi_math_cos((n - i - 1) * dgy) * pi_math_sin(j * dgx) * r;
				normal[(i * cels + j) * 3 + 1] = pos[(i * cels + j) * 3 + 1];

				pos[(i * cels + j) * 3 + 2] = pi_math_cos((n - i - 1) * dgy) * pi_math_cos(j * dgx) * r;
				normal[(i * cels + j) * 3 + 2] = pos[(i * cels + j) * 3 + 2];

			}

			

			if (i < rows - 1 && j < cels - 1)
			{
				index[(i * (cels - 1) + j) * 6] = (i * cels + j);
				index[(i * (cels - 1) + j) * 6 + 1] = (i * cels + j + 1);
				index[(i * (cels - 1) + j) * 6 + 2] = ((i + 1) * cels + j + 1);

				index[(i * (cels - 1) + j) * 6 + 3] = (i * cels + j);
				index[(i * (cels - 1) + j) * 6 + 4] = ((i + 1) * cels + j + 1);
				index[(i * (cels - 1) + j) * 6 + 5] = ((i + 1) * cels + j);
			}
		}
	}

	mesh = pi_mesh_create(EGOT_TRIANGLE_LIST, EINDEX_32BIT, rows * cels, pos, NULL, normal, tcoord, (rows - 1) * (cels - 1) * 2 * 3, index);
	pi_free(pos);
	pi_free(tcoord);
	pi_free(index);
	pi_free(normal);
	return mesh;
}

PiMesh* PI_API pi_mesh_create_sphere(float r)
{
	int n = 10;
	int rows = n * 2 + 1;
	int cels = n * 4 + 1;
	int i, j;
	float* tcoord = pi_new(float, rows * cels * 2);
	float* pos = pi_new(float, rows * cels * 3);
	float tcdx = 1.0f / (rows - 1);
	float tcdy = 1.0f / (cels - 1);
	float dgy = PI / (rows - 1);
	float dgx = 2 * PI / (cels - 1);
	PiMesh* mesh;
	uint32* index = pi_new(uint32, (rows - 1) * (cels - 1) * 2 * 3);

	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < cels; j++)
		{
			tcoord[(i * cels + j) * 2] = i * tcdx;
			tcoord[(i * cels + j) * 2 + 1] = j * tcdy;

			pos[(i * cels + j) * 3 + 1] = pi_math_sin((n - i)* dgy) * r;
			pos[(i * cels + j) * 3] = pi_math_cos((n - i) * dgy) * pi_math_sin(j * dgx) * r;
			pos[(i * cels + j) * 3 + 2] = pi_math_cos((n - i) * dgy) * pi_math_cos(j * dgx) * r;

			if (i < rows - 1 && j < cels - 1)
			{
				index[(i * (cels - 1) + j) * 6] = (i * cels + j);
				index[(i * (cels - 1) + j) * 6 + 1] = (i * cels + j + 1);
				index[(i * (cels - 1) + j) * 6 + 2] = ((i + 1) * cels + j + 1);

				index[(i * (cels - 1) + j) * 6 + 3] = (i * cels + j);
				index[(i * (cels - 1) + j) * 6 + 4] = ((i + 1) * cels + j + 1);
				index[(i * (cels - 1) + j) * 6 + 5] = ((i + 1) * cels + j);
			}
		}
	}

	mesh = pi_mesh_create(EGOT_TRIANGLE_LIST, EINDEX_32BIT, rows * cels, pos, NULL, pos, tcoord, (rows - 1) * (cels - 1) * 2 * 3, index);
	pi_free(pos);
	pi_free(tcoord);
	pi_free(index);
	return mesh;
}


PiMesh *PI_API pi_mesh_create_cube(float color[96], float uv[48])
{
	PiVector3 normal[24];
	float pos[24 * 3] =
	{
		-0.5f, -0.5f, -0.5f,  -0.5f, -0.5f, -0.5f,	-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,  -0.5f, -0.5f,  0.5f,	-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,  -0.5f,  0.5f, -0.5f,  -0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,
		0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f,
		0.5f, -0.5f,  0.5f,   0.5f, -0.5f,  0.5f,	 0.5f, -0.5f,  0.5f,
		0.5f,  0.5f, -0.5f,   0.5f,  0.5f, -0.5f,   0.5f,  0.5f, -0.5f,
		0.5f,  0.5f,  0.5f,   0.5f,  0.5f,  0.5f,   0.5f,  0.5f,  0.5f
	};

	float tcoord[24 * 2] =
	{
		0.0f, 0.0f,   1.0f, 0.0f,   1.0f, 1.0f,
		1.0f, 0.0f,   0.0f, 0.0f,   1.0f, 0.0f,
		0.0f, 1.0f,   0.0f, 0.0f,   0.0f, 1.0f,
		1.0f, 1.0f,   1.0f, 0.0f,   0.0f, 0.0f,
		1.0f, 1.0f,   1.0f, 1.0f,   1.0f, 0.0f,
		1.0f, 0.0f,   0.0f, 1.0f,   1.0f, 1.0f,
		0.0f, 1.0f,   0.0f, 1.0f,   0.0f, 0.0f,
		0.0f, 0.0f,   1.0f, 1.0f,   0.0f, 1.0f,
	};

	uint32 index[6 * 6] =
	{
		0,  3,  9,   0,  9,  6,
		21, 15, 12,  21, 12, 18,
		7, 10, 22,   7, 22, 19,
		4,  1, 13,   4, 13, 16,
		11,  5, 17,  11, 17, 23,
		20, 14,  2,  20,  2,  8
	};

	if (uv == NULL)
	{
		uv = tcoord;
	}

	/* 左面：原顶点 0, 1, 3, 2 */
	pi_vec3_set(&normal[0], -1.0f, 0.0f, 0.0f);
	pi_vec3_set(&normal[3], -1.0f, 0.0f, 0.0f);
	pi_vec3_set(&normal[6], -1.0f, 0.0f, 0.0f);
	pi_vec3_set(&normal[9], -1.0f, 0.0f, 0.0f);

	/* 右面：原顶点 7, 5, 4, 6 */
	pi_vec3_set(&normal[21], 1.0f, 0.0f, 0.0f);
	pi_vec3_set(&normal[15], 1.0f, 0.0f, 0.0f);
	pi_vec3_set(&normal[12], 1.0f, 0.0f, 0.0f);
	pi_vec3_set(&normal[18], 1.0f, 0.0f, 0.0f);

	/* 上面：原顶点 2, 3, 7, 6 */
	pi_vec3_set(&normal[7], 0.0f, 1.0f, 0.0f);
	pi_vec3_set(&normal[10], 0.0f, 1.0f, 0.0f);
	pi_vec3_set(&normal[22], 0.0f, 1.0f, 0.0f);
	pi_vec3_set(&normal[19], 0.0f, 1.0f, 0.0f);

	/* 下面：原顶点 1, 0, 4, 5 */
	pi_vec3_set(&normal[4], 0.0f, -1.0f, 0.0f);
	pi_vec3_set(&normal[1], 0.0f, -1.0f, 0.0f);
	pi_vec3_set(&normal[13], 0.0f, -1.0f, 0.0f);
	pi_vec3_set(&normal[16], 0.0f, -1.0f, 0.0f);

	/* 前面：原顶点 3, 1, 5, 7 */
	pi_vec3_set(&normal[11], 0.0f, 0.0f, 1.0f);
	pi_vec3_set(&normal[5], 0.0f, 0.0f, 1.0f);
	pi_vec3_set(&normal[17], 0.0f, 0.0f, 1.0f);
	pi_vec3_set(&normal[23], 0.0f, 0.0f, 1.0f);

	/* 后面，原顶点 6, 4, 0, 2 */
	pi_vec3_set(&normal[20], 0.0f, 0.0f, -1.0f);
	pi_vec3_set(&normal[14], 0.0f, 0.0f, -1.0f);
	pi_vec3_set(&normal[2], 0.0f, 0.0f, -1.0f);
	pi_vec3_set(&normal[8], 0.0f, 0.0f, -1.0f);

	return pi_mesh_create(EGOT_TRIANGLE_LIST, EINDEX_32BIT, 24, pos, color, normal, uv, 36, index);
}

void PI_API pi_mesh_set_index(PiMesh *mesh, PiBool is_copy, uint32 num, EIndexType type, EBufferUsage usage, void *index)
{
	if (mesh != NULL)
	{
		PiRenderData *data = &mesh->data;
		pi_renderdata_set_index(data, is_copy, num, type, usage, index);
	}
}

/*更新流数据*/
void PI_API pi_mesh_update_vertex(PiMesh* mesh, uint32 index, uint32 vertex_num, VertexSemantic semantic, uint32 src_stride, const void* vertex)
{
	if (mesh != NULL)
	{
		PiRenderData* data = &mesh->data;
		pi_renderdata_update_vertex(data, index, vertex_num, semantic, src_stride, vertex);
	}
}

/* 设置流数据 */
void PI_API pi_mesh_set_vertex(PiMesh *mesh, uint32 vertex_num,
                               PiBool is_copy, VertexSemantic semantic, uint32 num, EVertexType type, EBufferUsage usage, void *vertex)
{
	if (mesh != NULL)
	{
		PiRenderData *data = &mesh->data;
		pi_renderdata_set_vertex(data, vertex_num, is_copy, semantic, num, type, usage, vertex);
	}
}

uint PI_API pi_mesh_get_bone_num(PiMesh* mesh)
{
	return mesh->data.bone_num;
}
