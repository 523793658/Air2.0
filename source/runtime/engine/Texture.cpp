#include "Texture.h"
#include "boost/algorithm/string.hpp"
#include "PixelFormat.h"
#include "ResImporter/TextureImporter.h"
#include "Misc/Paths.h"
#include "Misc/App.h"
#include "TextureCube.h"
#include "Texture2D.h"
#include "SimpleReflection.h"
namespace Air
{

	RTexture::RTexture(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		if (App::canEverRender() && !isTemplate())
		{
			mTextureReference.beginInit_RenderThread();
		}
	}

	void RTexture::postLoad()
	{
		if (!isTemplate())
		{
			RTextureCube* cubeMap = dynamic_cast<RTextureCube*>(getOuter());
			if (cubeMap == nullptr)
			{
				updateResource();
			}
		}
	}

	void RTexture::updateResource()
	{
		releaseResource();
		if (App::canEverRender() && !hasAnyFlags(RF_ClassDefaultObject))
		{
			mResource = createResource();
			if (mResource)
			{
				beginInitResource(mResource);
			}
		}
	}

	void RTexture::releaseResource()
	{
		if (mResource)
		{
			releaseResourceAndFlush(mResource);
			delete mResource;
			mResource = nullptr;
		}
	}

	DECLARE_SIMPLER_REFLECTION(RTexture);
}