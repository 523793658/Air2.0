#pragma once
#include "EngineMininal.h"
#include "RenderingThread.h"
#include "Misc/Guid.h"
namespace Air
{
	class ENGINE_API LightMap : private DeferredCleanupInterface
	{
	public:
		enum
		{
			LMT_None = 0,
			LMT_1D = 1,
			LMT_2D = 2,
		};

		TArray<Guid> mLightGuids;
		LightMap();

		virtual ~LightMap() { BOOST_ASSERT(mNumRef == 0); }

		bool containsLight(const Guid& lightGuid) const
		{
			return mLightGuids.find(lightGuid) != mLightGuids.end();
		}


	protected:
		bool bAllowHighQualityLightMaps;

		virtual void cleanup();

	private:
		int32 mNumRef;

		virtual void finishCleanup();
	};
}