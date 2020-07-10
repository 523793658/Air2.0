#ifndef _Physx_Resource_Callback_H_
#define _Physx_Resource_Callback_H_
#include "ResourceCallback.h"
#include "Apex.h"
#include "string"
#include "vector"
#include "map"
#include "PsString.h"

using namespace nvidia;

#define PATH_MAX_LEN 512


class PiResourceCallback : public ResourceCallback
{
public:
	enum CustomResourceNameSpace
	{
		ShaderFilePath,
		TextureResource
	};
private:
	ApexSDK* mApexSDK;
	struct SearchDir
	{
		std::string path;
		bool recursive;
	};
	std::vector<SearchDir> mSearchDirs;
	std::map<std::pair<CustomResourceNameSpace, std::string>, void*> mCustomResource;
public:
	void setApexSDK(ApexSDK* sdk);
public:
	virtual ~PiResourceCallback();

	bool findFileInDir(const char* fileNameFull, const char* path, bool recursive, char* foundPath);
	bool findFile(const char* filename, std::vector<const char*> exts, char* foundPath);

	NvParameterized::Interface* deserializeFromFile(const char* name);

	/**
	\brief Request a resource from the user

	Will be called by the ApexSDK if a named resource is required but has not yet been provided.
	The resource pointer is returned directly, ResourceProvider::setResource() should not be called.
	This function will be called at most once per named resource, unless an intermediate call to
	releaseResource() has been made.

	\note If this call results in the application calling ApexSDK::createAsset, the name given
	to the asset must match the input name parameter in this method.
	*/
	virtual void* requestResource(const char* nameSpace, const char* name);
	/**
	\brief Request the user to release a resource

	Will be called by the ApexSDK when all internal references to a named resource have been released.
	If this named resource is required again in the future, a new call to requestResource() will be made.
	*/
	virtual void  releaseResource(const char* nameSpace, const char* name, void* resource);
	void addSearchDir(const char* dir, bool recursive);
};


#endif