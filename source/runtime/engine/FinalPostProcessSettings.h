#pragma once
#include "CoreMinimal.h"
#include "scene.h"
namespace Air
{
	

	class FinalPostProcessSettings : public PostProcessSettings
	{
	public:
		struct CubemapEntry
		{
			LinearColor mAmbientCubemapTintMulScaleValue;

			std::shared_ptr<class RTexture> mAmbientCubemap;

			CubemapEntry()
				:mAmbientCubemapTintMulScaleValue(LinearColor(0, 0, 0, 0))
				, mAmbientCubemap()
			{}
		};

		struct LUTBlenderEntry
		{
			float mWeight;
			class RTexture* mLUTexture;
		};

	public:
		TArray<CubemapEntry, TInlineAllocator<8>> mContributingCubemaps;
	};
}