#ifndef INCUDE_PARSE_H
#define INCUDE_PARSE_H
#include "image.h"
#include "audio_decoder.h"
#include "pi_mesh.h"
#include "pi_skeleton.h"
#include "vertex_anim.h"
#include "clut.h"
#include "uv_anim_controller.h"


/*
*��ΪImage�Ľ����������ת����js�������Ҫ�ֶ��ͷ�ImageResultָ��
*/
typedef struct
{
	PiImage* data;
	int type;
	int width;
	int height;
	int depth;
	int format;
	int numMipmap;
	int arraySize;

}ImageResult;


/*
*mesh�Ľ�������ṹ�塣ת����js�������Ҫ�ͷ�meshResultָ���meshesָ�루ֻ�ͷ�meshesָ�뱾���ڲ���Ա����Ҫ�ͷţ�
*���collision��ΪNULLҲ��Ҫ�ͷš�
*/
typedef struct
{
	PiMesh** meshes;
	void** collision;
	int count;
} meshResult; 

typedef struct 
{
	AudioWaveData* data;
	// �ļ���������
	int errorType;

	// �ļ�������Ϣ
	wchar_t* errorInfo;
}AudioResult;


/************************************************************************/
/* �ر�mesh�Ľ�������ṹ�����ж�����Ҫ�ֶ��ͷ�                       */
/************************************************************************/
typedef struct
{
	PiMesh* mesh;
	void* collision;
	float* posBuffer;
	float* norBuffer;
	float* colorBuffer;
	int posCount;
	int norCount;
	int colorCount;
}TerrainResult;

PI_BEGIN_DECLS

AudioDecodeResult PI_API app_load_audio_editor(wchar*path, AudioFormat format, AudioWaveData** data);

ImageResult* PI_API app_parse_load_image(byte* data, uint size, PiBool isDecompress);
void PI_API app_parse_image_result_free(ImageResult* result);

meshResult* PI_API app_parse_load_mesh(byte* data, uint size, PiBool createCollision);
void PI_API app_parse_mesh_result_free(meshResult* result);


TerrainResult* PI_API app_parse_load_terrain_mesh(byte* data, uint size, int vw, int vh, int gs, PiBool isEditor);
void PI_API app_parse_terrain_result_free(TerrainResult* result);

AudioResult* PI_API app_load_audio(wchar* path, AudioFormat format);
void app_load_audio_result_free(AudioResult* result);

PiSkeleton* PI_API app_parse_load_skeleton(byte* data, int size);

PiVertexAnim* PI_API app_parse_load_vertex_anim(byte* data, int size);

PiUVAnim* PI_API app_parse_load_uv_anim(byte* data, int size);

PiColorLookUpTable* PI_API app_parse_load_clut(byte* data, int size);

PiBool PI_API app_repx_to_ragdoll(wchar* repxPath, wchar* skPath, wchar* ragdollPath);

PI_END_DECLS


#endif