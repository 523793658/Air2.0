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
*作为Image的解析结果。在转换成js对象后需要手动释放ImageResult指针
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
*mesh的解析结果结构体。转换成js对象后需要释放meshResult指针和meshes指针（只释放meshes指针本身，内部成员不许要释放）
*如果collision不为NULL也需要释放。
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
	// 文件错误类型
	int errorType;

	// 文件错误信息
	wchar_t* errorInfo;
}AudioResult;


/************************************************************************/
/* 地表mesh的解析结果结构。所有对象都需要手动释放                       */
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