#include <vector>
#include "d3d9_mesh.h"
#include "d3d9_renderlayout.h"
#include "d3d9_convert.h"
#include "d3d9_rendersystem.h"
#include "d3d9_texture.h"

extern "C"{
	const BYTE x_rgcbFields[] =
	{
		1, // D3DDECLTYPE_FLOAT1,        // 1D float expanded to (value, 0., 0., 1.)
		2, // D3DDECLTYPE_FLOAT2,        // 2D float expanded to (value, value, 0., 1.)
		3, // D3DDECLTYPE_FLOAT3,       / 3D float expanded to (value, value, value, 1.)
		4, // D3DDECLTYPE_FLOAT4,       / 4D float
		4, // D3DDECLTYPE_D3DCOLOR,      // 4D packed unsigned bytes mapped to 0. to 1. range
		//                      // Input is in D3DCOLOR format (ARGB) expanded to (R, G, B, A)
		4, // D3DDECLTYPE_UBYTE4,        // 4D unsigned byte
		2, // D3DDECLTYPE_SHORT2,        // 2D signed short expanded to (value, value, 0., 1.)
		4, // D3DDECLTYPE_SHORT4         // 4D signed short

		4, // D3DDECLTYPE_UBYTE4N,       // Each of 4 bytes is normalized by dividing to 255.0
		2, // D3DDECLTYPE_SHORT2N,       // 2D signed short normalized (v[0]/32767.0,v[1]/32767.0,0,1)
		4, // D3DDECLTYPE_SHORT4N,       // 4D signed short normalized (v[0]/32767.0,v[1]/32767.0,v[2]/32767.0,v[3]/32767.0)
		2, // D3DDECLTYPE_USHORT2N,      // 2D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,0,1)
		4, // D3DDECLTYPE_USHORT4N,      // 4D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,v[2]/65535.0,v[3]/65535.0)
		3, // D3DDECLTYPE_UDEC3,         // 3D unsigned 10 10 10 format expanded to (value, value, value, 1)
		3, // D3DDECLTYPE_DEC3N,         // 3D signed 10 10 10 format normalized and expanded to (v[0]/511.0, v[1]/511.0, v[2]/511.0, 1)
		2, // D3DDECLTYPE_FLOAT16_2,     // 2D 16 bit float expanded to (value, value, 0, 1 )
		4, // D3DDECLTYPE_FLOAT16_4,     // 4D 16 bit float 
		0, // D3DDECLTYPE_UNKNOWN,       // Unknown
	};

	typedef struct _D3D9Mesh
	{
		ID3DXMesh* impl;
		D3DVERTEXELEMENT9 elements[EVS_NUM];
		uint num[EVS_NUM];
		EVertexType type[EVS_NUM];
		uint strides[EVS_NUM];
		void* data[EVS_NUM];
		WORD stride;
		uint num_elem;
	}D3D9Mesh;

	extern PiRenderSystem *g_rsystem;

	static PiSelectR PI_API _gen_vertex_element(D3D9Mesh *user_data, VertexElement *value)
	{
		uint index = user_data->num_elem++;
		D3DVERTEXELEMENT9* ele = &user_data->elements[index];
		ele->Stream = EVS_POSITION;
		ele->Offset = user_data->stride;
		d3d9_vertex_type_get(value->type, value->num, &ele->Type, &user_data->strides[index]);
		user_data->type[index] = value->type;
		user_data->num[index] = value->num;
		ele->Method = D3DDECLMETHOD_DEFAULT;
		d3d9_vertex_semantic_get(value->semantic, &ele->Usage, &ele->UsageIndex);
		user_data->data[index] = value->data;
		user_data->stride += (WORD)user_data->strides[index];
		return SELECT_NEXT;
	}

	static VertexSemantic from_d3d9_semantic(D3DDECLUSAGE usage, uint usageIndex)
	{
		VertexSemantic vs = EVS_TEXCOORD_0;
		switch (usage)
		{
		case D3DDECLUSAGE_POSITION:
			vs = EVS_POSITION;
			break;
		case D3DDECLUSAGE_NORMAL:
			vs = EVS_NORMAL;
			break;
		case D3DDECLUSAGE_COLOR:
			if (usageIndex == 0){
				vs = EVS_DIFFUSE;
			}
			else if (usageIndex == 1){
				vs = EVS_SPECULAR;
			}
			break;
		case D3DDECLUSAGE_BINORMAL:
			vs = EVS_BINORMAL;
			break;
		case D3DDECLUSAGE_TANGENT:
			vs = EVS_TANGENT;
			break;
		case D3DDECLUSAGE_BLENDWEIGHT:
			vs = EVS_BLEND_WEIGHTS;
			break;
		case D3DDECLUSAGE_BLENDINDICES:
			vs = EVS_BLEND_INDICES;
			break;
		case D3DDECLUSAGE_TEXCOORD:
		{
			switch (usageIndex)
			{
			case 0:
				vs = EVS_TEXCOORD_0;
				break;
			case 1:
				vs = EVS_TEXCOORD_1;

				break; 
			case 2:
				vs = EVS_TEXCOORD_2;

				break; 
			case 3:
				vs = EVS_TEXCOORD_3;

				break; 
			case 4:
				vs = EVS_TEXCOORD_4;

				break; 
			case 5:
				vs = EVS_TEXCOORD_5;

				break; 
			case 6:					
				vs = EVS_TEXCOORD_6;

				break; 
			case 7:
				vs = EVS_INSTANCE;
				break;
			default:
				break;
			}
		}
		default:
			break;
		}
		return vs;
	}

	D3D9Mesh* PI_API d3d9_mesh_create(PiMesh* mesh)
	{
		D3D9Mesh* d3d9_mesh = pi_new0(D3D9Mesh, 1);
		uint numFaces = pi_mesh_get_face_num(mesh);
		uint numVertices = pi_mesh_get_vertex_num(mesh);
		D3D9RenderSystem* d3d9_rendersystem = (D3D9RenderSystem*)g_rsystem->impl;
		D3D9Context* context = d3d9_rendersystem->context;
		pi_dhash_foreach(&mesh->data.vertex_map, (PiSelectFunc)_gen_vertex_element, d3d9_mesh);
		d3d9_mesh->elements[d3d9_mesh->num_elem] = D3DDECL_END();
		HRESULT hr = D3DXCreateMesh(numFaces, numVertices, 0, d3d9_mesh->elements, context->device, &d3d9_mesh->impl);
		void* vertex_data;
		hr = d3d9_mesh->impl->LockVertexBuffer(D3DLOCK_DISCARD, &vertex_data);
		for (uint i = 0; i < numVertices; i++)
		{
			uint size = 0;
			for (uint j = 0; j < d3d9_mesh->num_elem; j++)
			{
				pi_memcpy_inline((byte*)vertex_data + d3d9_mesh->stride * i + size, (byte*)d3d9_mesh->data[j] + d3d9_mesh->strides[j] * i, d3d9_mesh->strides[j]);
				size += d3d9_mesh->strides[j];
			}
		}

		hr = d3d9_mesh->impl->UnlockVertexBuffer();

		void* index_data;
		hr = d3d9_mesh->impl->LockIndexBuffer(D3DLOCK_DISCARD, &index_data);
		if (mesh->data.idata.type == EINDEX_32BIT)
		{
			for (uint i = 0; i < mesh->data.idata.num; i++)
			{
				*((uint16*)index_data + i) = (uint16)(*((uint32*)mesh->data.idata.data + i));
			}
		}
		else
		{
			pi_memcpy_inline(index_data, mesh->data.idata.data, mesh->data.idata.num * sizeof(uint16));
		}

		d3d9_mesh->impl->UnlockIndexBuffer();
		return d3d9_mesh;
	}

	HRESULT _computeIMTCallback(FLOAT fPercentDone, LPVOID lpUserContext)
	{
		return S_OK;
	}

	static inline CONST D3DVERTEXELEMENT9* GetDeclElement(CONST D3DVERTEXELEMENT9* pDecl, D3DDECLUSAGE Usage, DWORD UsageIndex)
	{
		while (pDecl->Stream != 0xff)
		{
			if ((pDecl->Usage == Usage) && pDecl->UsageIndex == UsageIndex)
			{
				return pDecl;
			}
			pDecl++;
		}
		return NULL;
	}
	static inline UINT getFields(CONST D3DVERTEXELEMENT9* pElement)
	{
		if (pElement->Type <= D3DDECLTYPE_FLOAT16_4)
			return x_rgcbFields[pElement->Type];
		return 0;
	}

	HRESULT PerVertexIMT(ID3DXMesh* pMesh, CONST D3DVERTEXELEMENT9 *pDecal, D3DDECLUSAGE usage, DWORD usageIndex, ID3DXBuffer **ppIMTData)
	{
		UINT uSignalDimension;
		HRESULT hr;
		float* pVertexData = NULL;

		const D3DVERTEXELEMENT9* element;
		element = GetDeclElement(pDecal, usage, usageIndex);
		uSignalDimension = getFields(element);
		pMesh->LockVertexBuffer(D3DLOCK_NOSYSLOCK, (void**)&pVertexData);


		hr = D3DXComputeIMTFromPerVertexSignal(pMesh, pVertexData + element->Offset, uSignalDimension, pMesh->GetNumBytesPerVertex(), 0L, NULL, NULL, ppIMTData);
		if (pVertexData){
			pMesh->UnlockVertexBuffer();
		}
		return hr;
	}

	PiMesh* PI_API mesh_atlas_create(PiMesh* mesh)
	{
		D3D9Mesh* d3d9_mesh = d3d9_mesh_create(mesh);
		DWORD* pdwAdjacency = (DWORD*)pi_malloc(sizeof(DWORD) * d3d9_mesh->impl->GetNumFaces() * 3);
		BYTE* vertexData;
		HRESULT hr = d3d9_mesh->impl->GenerateAdjacency(0.0f, pdwAdjacency);
		//ID3DXBuffer *IMTBuffer;
		D3DVERTEXELEMENT9 decl[MAX_FVF_DECL_SIZE];
		d3d9_mesh->impl->GetDeclaration(decl);
		//PerVertexIMT(d3d9_mesh->impl, decl, D3DDECLUSAGE_POSITION, 0, &IMTBuffer);

		ID3DXMesh *outputMesh;
		ID3DXBuffer *ppVertexRemapArray;
		float pfMaxStretchOut;
		uint pdwNumChartsOut;
		//DWORD size = IMTBuffer->GetBufferSize();
		//pi_log_print(LOG_INFO, "size:%d", size);
		hr = D3DXUVAtlasCreate(d3d9_mesh->impl, 0, 1.0f, 512, 512, 3, 0, pdwAdjacency, NULL ,NULL,//(float*)IMTBuffer->GetBufferPointer(), 
			NULL, 0.01f, NULL, D3DXUVATLAS_GEODESIC_QUALITY, &outputMesh,
			NULL, &ppVertexRemapArray, &pfMaxStretchOut, &pdwNumChartsOut);
		//IMTBuffer->Release();
		pi_free(pdwAdjacency);
		uint vertexNum = outputMesh->GetNumVertices();
		DWORD* vertexMapData = (DWORD*)ppVertexRemapArray->GetBufferPointer();
		outputMesh->LockVertexBuffer(0, (LPVOID*)&vertexData);
		void** newVertexData = pi_new0(void*, d3d9_mesh->num_elem + 1);
		for (uint i = 0; i < d3d9_mesh->num_elem; i++)
		{
			newVertexData[i] = pi_malloc(d3d9_mesh->strides[i] * vertexNum);
		}
		uint floatSize = sizeof(float);
		newVertexData[d3d9_mesh->num_elem] = pi_malloc(2 * sizeof(float) * vertexNum);
		uint16 minBoneIndex = 1000, maxBoneIndex = 0;
		for (uint i = 0; i < vertexNum; i++)
		{
			byte* vertexDataOffsetPoint = ((byte*)vertexData) + d3d9_mesh->stride * i;
			uint offset = 0;
			byte* oldUVData = NULL;
			DWORD oldVertexIndex = vertexMapData[i];

			for (uint j = 0; j < d3d9_mesh->num_elem; j++)
			{
				if (d3d9_mesh->elements[j].Usage == D3DDECLUSAGE_TEXCOORD && d3d9_mesh->elements[j].UsageIndex == 0)
				{
					oldUVData = (byte*)d3d9_mesh->data[j];
				}
				if (d3d9_mesh->elements[j].Usage == D3DDECLUSAGE_TANGENT
					|| d3d9_mesh->elements[j].Usage == D3DDECLUSAGE_BLENDWEIGHT
					|| d3d9_mesh->elements[j].Usage == D3DDECLUSAGE_BLENDINDICES)
				{
					if (d3d9_mesh->elements[j].Usage == D3DDECLUSAGE_BLENDINDICES && mesh->version < 3)
					{
						uint16* indices = (uint16*)((byte*)d3d9_mesh->data[j] + d3d9_mesh->strides[j] * oldVertexIndex);
						for (uint k = 0; k < 4; k++)
						{
							minBoneIndex = min(minBoneIndex, *(indices + k));
							maxBoneIndex = max(maxBoneIndex, *(indices + k));
						}
					}
					pi_memcpy_inline(((byte*)newVertexData[j]) + d3d9_mesh->strides[j] * i, (byte*)d3d9_mesh->data[j] + d3d9_mesh->strides[j] * oldVertexIndex, d3d9_mesh->strides[j]);
				}
				else
				{
					pi_memcpy_inline(((byte*)newVertexData[j]) + d3d9_mesh->strides[j] * i, vertexDataOffsetPoint + offset, d3d9_mesh->strides[j]);
				}
				offset += d3d9_mesh->strides[j];
			}
			pi_memcpy_inline(((byte*)newVertexData[d3d9_mesh->num_elem]) + 2 * floatSize * i, oldUVData + 2 * floatSize * oldVertexIndex, 2 * floatSize);
		}
		ppVertexRemapArray->Release();
		outputMesh->UnlockVertexBuffer();
		void* indexData;
		hr = outputMesh->LockIndexBuffer(0, &indexData);
		uint indexNum = outputMesh->GetNumFaces() * 3;
		uint32 *newIndexData = pi_new0(uint32, indexNum);
		for (uint i = 0; i < indexNum; i++)
		{
			newIndexData[i] = (uint32)(((uint16*)indexData)[i]);
		}


		PiMesh* newMesh = pi_mesh_create(EGOT_TRIANGLE_LIST, EINDEX_32BIT, vertexNum, NULL, NULL, NULL, NULL, outputMesh->GetNumFaces() * 3, newIndexData);
		outputMesh->UnlockIndexBuffer();
		pi_free(newIndexData);
		for (uint i = 0; i < d3d9_mesh->num_elem; i++)
		{
			VertexSemantic evs = from_d3d9_semantic((D3DDECLUSAGE)d3d9_mesh->elements[i].Usage, d3d9_mesh->elements[i].UsageIndex);
			if (evs == EVS_TEXCOORD_1){
				continue;
			}
			else if (evs == EVS_TEXCOORD_0)
			{
				evs = EVS_TEXCOORD_1;
			}
			pi_mesh_set_vertex(newMesh, vertexNum, TRUE, evs, d3d9_mesh->num[i], d3d9_mesh->type[i], EVU_STATIC_DRAW, newVertexData[i]);
		}
		pi_mesh_set_vertex(newMesh, vertexNum, TRUE, EVS_TEXCOORD_0, 2, EVT_FLOAT, EVU_STATIC_DRAW, newVertexData[d3d9_mesh->num_elem]);
		for (uint i = 0; i < d3d9_mesh->num_elem + 1; i++)
		{
			pi_free(newVertexData[i]);
		}
		pi_free(newVertexData);
		d3d9_mesh->impl->Release();
		outputMesh->Release();
		pi_free(d3d9_mesh);
		newMesh->data.bone_offset = mesh->data.bone_offset;
		newMesh->data.bone_num = mesh->data.bone_num;
		if (mesh->version < 3 && maxBoneIndex >= minBoneIndex)
		{
			newMesh->data.bone_offset = (float)minBoneIndex;
			newMesh->data.bone_num = maxBoneIndex - minBoneIndex + 1;
		}
		return newMesh;
	}
}


