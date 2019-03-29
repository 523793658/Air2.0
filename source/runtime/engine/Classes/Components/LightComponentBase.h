#pragma once
#include "CoreMinimal.h"
#include "Classes/Components/SceneComponent.h"
#include "misc/Guid.h"
namespace Air
{
	class ENGINE_API LightComponentBase : public SceneComponent
	{
		GENERATED_RCLASS_BODY(LightComponentBase, SceneComponent)

	public:
		bool hasStaticLighting() const;

		bool hasStaticShadow() const;

		bool isMovable() const
		{
			return (mMobility == EComponentMobility::Movable);
		}
	public:
		Guid mLightGuid;

		//总的辐射能量
		float mIntensity;

		Color mLightColor;

		uint32 bAffectsWorld : 1;

		uint32 bCastShadows : 1;

		uint32 bCastStaticShadows : 1;

		uint32 bCastDynamicShadows : 1;

		uint32 bAffectTranslucentLighting : 1;

		float mIndirectLightingIntensity;

	};
}