#include "physx_resource_callback.h"
#include "DestructibleAsset.h"
#include "ClothingAsset.h"
#include "EmitterAsset.h"
#include "ImpactEmitterAsset.h"
#include "IofxAsset.h"
#include "BasicIosAsset.h"
#include "ParticleIosAsset.h"
#include "UserOpaqueMesh.h"
#include "material.h"
#include "pi_lib.h"
void PiResourceCallback::setApexSDK(ApexSDK* sdk)
{
	mApexSDK = sdk;
}
PiResourceCallback::~PiResourceCallback() {}

bool PiResourceCallback::findFileInDir(const char* fileNameFull, const char* path, bool recursive, char* foundPath)
{
	char tmp[PATH_MAX_LEN];
	shdfnd::snprintf(tmp, size_t(tmp), "%s\\%s", path, fileNameFull);
	PiFileInfo info;
	wchar * x = pi_str_to_wstr(tmp, PI_CP_UTF8);
	pi_vfs_get_info(x, &info);
	shdfnd::snprintf(foundPath, PATH_MAX_LEN, "%s", tmp);
	if (info.type == FILE_TYPE_REGULAR)
	{
		return true;
	}
	return true;
}

bool PiResourceCallback::findFile(const char* filename, std::vector<const char*> exts, char* foundPath)
{
	std::string fileNameOnly = filename;
	size_t ind = fileNameOnly.find_last_of('/');
	if (ind > 0)
	{
		fileNameOnly = fileNameOnly.substr(ind + 1);
	}
	for (size_t i = 0; i < mSearchDirs.size(); i++)
	{
		const SearchDir& searchDir = mSearchDirs[i];
		for (size_t j = 0; j < exts.size(); j++)
		{
			const char* ext = exts[j];
			const uint32_t fileMaxLen = 128;
			char fileNameFull[fileMaxLen] = { 0 };
			nvidia::shdfnd::snprintf(fileNameFull, fileMaxLen, "%s.%s", fileNameOnly.c_str(), ext);
			if (findFileInDir(fileNameFull, searchDir.path.c_str(), searchDir.recursive, foundPath))
			{
				return true;
			}
		}
		if (findFileInDir(fileNameOnly.c_str(), searchDir.path.c_str(), searchDir.recursive, foundPath))
		{
			return true;
		}
	}
	return false;
}

NvParameterized::Interface* PiResourceCallback::deserializeFromFile(const char* name)
{
	NvParameterized::Interface* params = NULL;
	char path[PATH_MAX_LEN];
	const char* exts[] = { "apb", "apx" };
	if (findFile(name, std::vector<const char*>(exts, exts + sizeof(exts) / sizeof(exts[0])), path))
	{
		PxFileBuf* stream = mApexSDK->createStream(path, PxFileBuf::OPEN_READ_ONLY);
		if (stream)
		{
			NvParameterized::Serializer::SerializeType serType = mApexSDK->getSerializeType(*stream);
			NvParameterized::Serializer::ErrorType serError;
			NvParameterized::Serializer* ser = mApexSDK->createSerializer(serType);
			PX_ASSERT(ser);
			NvParameterized::Serializer::DeserializedData data;
			serError = ser->deserialize(*stream, data);
			if (serError == NvParameterized::Serializer::ERROR_NONE && data.size() == 1)
			{
				params = data[0];
			}
			else
			{
				PX_ASSERT(0 && "ERROR Deserializing NvParameterized Asset");
			}
			stream->release();
			ser->release();
		}
	}
	return params;
}

void* PiResourceCallback::requestResource(const char* nameSpace, const char* name)
{
	Asset* asset = NULL;
	if (!shdfnd::strcmp(nameSpace, DESTRUCTIBLE_AUTHORING_TYPE_NAME)
		|| !shdfnd::strcmp(nameSpace, CLOTHING_AUTHORING_TYPE_NAME)
		|| !shdfnd::strcmp(nameSpace, EMITTER_AUTHORING_TYPE_NAME)
		|| !shdfnd::strcmp(nameSpace, IMPACT_EMITTER_AUTHORING_TYPE_NAME)
		|| !shdfnd::strcmp(nameSpace, IOFX_AUTHORING_TYPE_NAME)
		|| !shdfnd::strcmp(nameSpace, BASIC_IOS_AUTHORING_TYPE_NAME)
		|| !shdfnd::strcmp(nameSpace, PARTICLE_IOS_AUTHORING_TYPE_NAME)
		|| !shdfnd::strcmp(nameSpace, RENDER_MESH_AUTHORING_TYPE_NAME))
	{
		NvParameterized::Interface* params = deserializeFromFile(name);
		if (params != NULL)
		{
			asset = mApexSDK->createAsset(params, name);
			PX_ASSERT(asset && "ERROR Creating NvParameterized Asset");
		}
	}
	else if (!shdfnd::strcmp(nameSpace, APEX_OPAQUE_MESH_NAME_SPACE))
	{
		NvParameterized::Interface* params = deserializeFromFile("woodChunkMesh");
		if (params != NULL)
		{
			asset = mApexSDK->createAsset(params, "woodChunkMesh");
			PX_ASSERT(asset && "ERROR Creating NvParameterized Asset");
		}
	}
	else if (!shdfnd::strcmp(nameSpace, APEX_MATERIALS_NAME_SPACE))
	{
		char path[PATH_MAX_LEN];
		const char* exts[] = { "xml" };
		if (findFile(name, std::vector<const char*>(exts, exts + sizeof(exts) / sizeof(exts[0])), path))
		{
			//正式版在些材质管理
			PiMaterial* material = pi_material_new("skeleton.vs", "simplest.fs");
			pi_material_set_def(material, "SKELETON", TRUE);
			return material;
		}
	}
	return asset;
}

void  PiResourceCallback::releaseResource(const char* nameSpace, const char* name, void* resource)
{

}

void PiResourceCallback::addSearchDir(const char* dir, bool recursive)
{
	SearchDir searcDir = { dir, recursive };
	mSearchDirs.push_back(searcDir);
}
