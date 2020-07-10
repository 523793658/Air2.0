#include "parse.h"
#include "pi_mesh.h"
#include "pi_collision.h"
#include "SlimXml.h"

static void *load_file(wchar* filePath, uint *size)
{
	void *data = NULL;
	void *file = pi_file_open(filePath, FILE_OPEN_READ);
	*size = 0;
	if (file != NULL)
	{
		int64 size64 = 0;
		pi_file_size(file, &size64);
		*size = (uint)size64;
		data = pi_malloc0(*size);
		pi_file_read(file, 0, FALSE, (char *)data, *size);
		pi_file_close(file);
	}
	return data;
}

ImageResult* PI_API app_parse_load_image(byte* data, uint size, PiBool isDecompress)
{
	ImageResult* result;
	PiImage* image = pi_render_image_load(data, size, isDecompress);
	result = pi_new0(ImageResult, 1);
	if(image == 0)
	{
		return 0;
	}
	else
	{
		result->type = pi_render_image_get_type(image);
		result->width = pi_render_image_get_width(image);
		result->height = pi_render_image_get_height(image);
		result->depth = pi_render_image_get_depth(image);
		result->format = pi_render_image_get_format(image);
		result->numMipmap = pi_render_image_get_num_mipmap(image);
		result->arraySize = pi_render_image_get_array_size(image);
		result->data = image;
	}
	return result;
}

void PI_API app_parse_image_result_free(ImageResult* result)
{
	pi_free(result);
}

AudioResult* PI_API app_load_audio(wchar* path, AudioFormat format)
{
	AudioResult* result = pi_new0(AudioResult, 1);
	AudioWaveData* data;
	AudioDecodeResult adr = pi_audio_decode(path, format, &data);
	result->data = data;
	if (adr != ADR_OK)
	{
		if (adr == ADR_FILE_NOT_EXIST)
		{
			result->errorType = pi_error_get_type();
			result->errorInfo = pi_error_get_message();
		}
		else
		{
			result->errorType = 1;
			result->errorInfo = pi_wstr_dup(L"ÒôÆµ½âÂë´íÎó");
		}
	}
	return result;
}

AudioDecodeResult PI_API app_load_audio_editor(wchar*path, AudioFormat format, AudioWaveData** data)
{
	AudioDecodeResult adr = pi_audio_decode(path, format, data);
	return adr;
}

void app_load_audio_result_free(AudioResult* result)
{
	if (result->errorInfo)
	{
		pi_free(result->errorInfo);
	}
	pi_free(result);
}


meshResult* PI_API app_parse_load_mesh(byte* data, uint size, PiBool createCollision)
{
	int i;
	int meshNum;
	PiMesh* mesh;
	meshNum = pi_mesh_num(data, size);
	PiMesh** meshes = pi_new0(PiMesh*, meshNum);
	meshResult* result;
	void** collision;
	result = pi_new0(meshResult, 1);
	for(i = 0 ; i < meshNum; i++)
	{
		mesh = pi_mesh_create_empty();
		meshes[i] = mesh;
	}
	result->meshes = meshes;
	pi_mesh_load(meshes, meshNum, data, size);
	if(createCollision)
	{
		collision = pi_new0(void*, meshNum);
		for(i = 0 ; i < meshNum; ++i)
		{
			collision[i] = pi_collision_obj_new(mesh);
		}
		result->collision = collision;
	}
	result->count = meshNum;
	return result;
}

void PI_API app_parse_mesh_result_free(meshResult* result)
{
	pi_free(result->meshes);
	if (result->collision)
	{
		pi_free(result->collision);
	}
	pi_free(result);
}

TerrainResult* PI_API app_parse_load_terrain_mesh(byte* data, uint size, int vw, int vh, int gs, PiBool isEditor)
{
	float* posBuffer;
	PiBytes* posView;
	float* norBuffer;
	PiBytes* norView;
	float* colorBuffer;
	PiBytes* colorView;
	float* texCoordBuffer;
	PiBytes* texCoordView;
	int* indexBuffer;
	PiBool hasColor = TRUE;

	TerrainResult* result = pi_new0(TerrainResult, 1);
	PiMesh * mesh;

	uint8 temp;
	int indexNum;
	int ni, p1, p2, p3, p4;
	int i, j, ind, norOffset, colorOffset, texCoordOffset;

	if(vw * vh * 15 == size)
	{
		hasColor = FALSE;
	}

	posBuffer = pi_new(float, 3 * vw * vh);
	posView = pi_bytes_new();
	pi_bytes_load(posView, data, 3 * vw * vh * sizeof(float), FALSE);

	norOffset = 3 * vw * vh * sizeof(float);
	norView = pi_bytes_new();
	norBuffer = pi_new(float, 3 * vw * vh);
	pi_bytes_load(norView, (data + norOffset), 3 * vw * vh, FALSE);

	colorBuffer = pi_new(float, 4 * vw * vh);
	if(hasColor)
	{
		colorOffset = norOffset + 3 * vw * vh;
		pi_memcpy(colorBuffer, (data + colorOffset), 4 * vw * vh * sizeof(float));
	}
	else
	{
		pi_memset(colorBuffer, 1, 4 * vw * vh * sizeof(float));
	}
	



	texCoordBuffer = pi_new(float, 2 * vw * vh);
	
	indexNum = (vw - 1) * (vh - 1) * 6;
	indexBuffer = pi_new(int, indexNum);
	for(i = 0 , ind = 0 ; i < vw; i++)
	{
		for(j = 0 ; j < vh; j++, ind += 3)
		{
			pi_bytes_read_float(posView, &posBuffer[ind]);
			posBuffer[ind] += gs * i;
			
			pi_bytes_read_float(posView, &posBuffer[ind+ 1]);

			pi_bytes_read_float(posView, &posBuffer[ind+2]);
			posBuffer[ind+2] += gs * j;

			pi_bytes_read_uint8(norView, &temp);
			norBuffer[ind] = temp / 255.0f * 2.0f - 1.0f;

			pi_bytes_read_uint8(norView, &temp);
			norBuffer[ind + 1] = temp / 255.0f * 2.0f - 1.0f;

			pi_bytes_read_uint8(norView, &temp);
			norBuffer[ind + 2] = temp / 255.0f * 2.0f - 1.0f;

			texCoordBuffer[ind / 3 * 2] = i / (vw - 1.0);
			texCoordBuffer[ind / 3 * 2 + 1] = j / (vh - 1.0);
		}
	}

	ni = 0;
	for (i = 0; i < vw; i++) {
		for (j = 0; j < vh; j++) {
			if (i < vw - 1 && j < vh - 1) {
				p1 = i * vh + j;
				p2 = i * vh + (j + 1);
				p3 = (i + 1) * vh + j;
				p4 = (i + 1) * vh + (j + 1);
				indexBuffer[ni++] = p1;
				indexBuffer[ni++] = p2;
				indexBuffer[ni++] = p4;
				indexBuffer[ni++] = p1;
				indexBuffer[ni++] = p4;
				indexBuffer[ni++] = p3;
			}
		}
	}
	mesh = pi_mesh_create(EGOT_TRIANGLE_LIST, EINDEX_32BIT, vw * vh, posBuffer, colorBuffer, norBuffer, texCoordBuffer, indexNum, indexBuffer);
	result->mesh = mesh;
	result->collision = pi_collision_obj_new(mesh);
	if(isEditor){
		result->posBuffer = posBuffer;
		result->norBuffer = norBuffer;
		result->colorBuffer = colorBuffer;
		result->posCount = 3 * vw * vh;
		result->norCount = 3 * vw * vh;
		result->colorCount = 4 * vw * vh;
	} 
	else
	{
		pi_free(posBuffer);
		pi_free(norBuffer);
		pi_free(colorBuffer);
	}
	pi_free(texCoordBuffer);
	pi_free(indexBuffer);
	return result;
}
void PI_API app_parse_terrain_result_free(TerrainResult* result)
{
	if (result->posBuffer)
	{
		pi_free(result->posBuffer);
		pi_free(result->norBuffer);
		pi_free(result->colorBuffer);
	}
	pi_free(result);
}

PiSkeleton* PI_API app_parse_load_skeleton(byte* data, int size)
{
	return pi_skeleton_new(data, size);
}

PiVertexAnim* PI_API app_parse_load_vertex_anim(byte* data, int size)
{
	return pi_veanim_new(data, size);
}

PiUVAnim* PI_API app_parse_load_uv_anim(byte* data, int size)
{
	return app_uv_anim_new(data, size);

}

PiColorLookUpTable* PI_API app_parse_load_clut(byte* data, int size)
{
	return pi_clut_load(data, size);
}

PiBool PI_API app_repx_to_ragdoll(wchar* repxPath, wchar* skPath, wchar* ragdollPath)
{
	using namespace slim;
	XmlDocument doc;
	uint repx_size;
	char* repx_data = (char*)load_file(repxPath, &repx_size);
	if (repx_data == NULL)
	{
		return FALSE;
	}
	doc.loadFromMemory(repx_data, repx_size);

	uint sk_size;
	byte *sk_data = (byte *)load_file(skPath, &sk_size);
	if (sk_data == NULL)
	{
		pi_free(repx_data);
		return FALSE;
	}
	PiSkeleton* sk = pi_skeleton_new(sk_data, sk_size);

	NodeIterator it;
	uint32 nodeCount = 0;
	PiBytes* buffer = pi_bytes_new();
	pi_bytes_write_int(buffer, nodeCount);
	XmlNode* PhysX30Collection = doc.findFirstChild(L"PhysX30Collection", it);
	XmlNode* node = PhysX30Collection->findFirstChild(L"PxRigidDynamic", it);
	while (node != nullptr)
	{
		XmlNode* idNode = node->findChild(L"Id");
		int actorId = idNode->getInt();
		XmlNode* nameNode = node->findChild(L"Name");
		wchar* name = (wchar*)(nameNode->getString());
		int boneId = pi_skeleton_get_bone_id(sk, name);
		uint32 nameLength = wcslen(name);
		uint32 nameSize = (nameLength + 1) * sizeof(wchar_t);
		pi_bytes_write_wstr(buffer, name);
		//fwrite(&nameSize, sizeof(uint32), 1, file);
		//fwrite(name, sizeof(wchar_t), nameLength + 1, file);
		pi_bytes_write_int(buffer, actorId);
		pi_bytes_write_int(buffer, boneId);

		//fwrite(&actorId, sizeof(int), 1, file);
		//fwrite(&boneId, sizeof(int), 1, file);
		nodeCount++;
		node = PhysX30Collection->findNextChild(L"PxRigidDynamic", it);
	}
	int index = pi_bytes_windex(buffer, 0);
	pi_bytes_write_int(buffer, nodeCount);
	pi_bytes_windex(buffer, index);

	pi_bytes_write_binary(buffer, repx_data, repx_size);

	void* handle = pi_file_open(ragdollPath, FILE_OPEN_WRITE);
	pi_file_write(handle, 0, FALSE, (char*)pi_bytes_array(buffer, 0), pi_bytes_size(buffer));
	pi_file_close(handle);

	pi_bytes_free(buffer);
	pi_skeleton_free(sk);
	pi_free(sk_data);
	pi_free(repx_data);
	return TRUE;
}