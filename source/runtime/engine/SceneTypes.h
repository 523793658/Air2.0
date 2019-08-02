#pragma once
#include "EngineMininal.h"
#include "Containers/LinkList.h"
namespace Air
{
	class SceneViewStateInterface;

	inline uint32 getTypeHash(const uint32 A)
	{
		return A;
	}



	class PrimitiveComponentId
	{
	public:
		PrimitiveComponentId() : mPrimIdValue(0)
		{}

		inline bool isValid() const
		{
			return mPrimIdValue > 0;
		}

		inline bool operator == (PrimitiveComponentId rhs) const
		{
			return mPrimIdValue == rhs.mPrimIdValue;
		}

		friend uint32 getTypeHash(PrimitiveComponentId id)
		{
			return getTypeHash(id.mPrimIdValue);
		}
		uint32 mPrimIdValue;
	};

	class SceneViewStateReference
	{
	public:
		SceneViewStateReference():
			mReference(nullptr)
		{}

		ENGINE_API virtual ~SceneViewStateReference();

		ENGINE_API void allocate();

		ENGINE_API void destroy();

		ENGINE_API static void destroyAll();
		ENGINE_API static void allocateAll();

		SceneViewStateInterface* getReference()
		{
			return mReference;
		}



	private:
		SceneViewStateInterface* mReference;

		TLinkedList<SceneViewStateReference*> mGlobalListLink;

		static TLinkedList<SceneViewStateReference*>*& getSceneViewStateList();
	};

	enum ELightMapInteractionType
	{
		LMIT_None = 0,
		LMIT_Texture =2,
		LMIT_NumBits = 3
	};

	enum EMaterialProperty
	{
		MP_BaseColor,
		MP_Normal,
		MP_Metallic,
		MP_Specular,
		MP_Roughness,
		MP_AmbientOcclusion,
		MP_EmissiveColor,
		MP_CustomizedUVs0,
		MP_CustomizedUVs1,
		MP_CustomizedUVs2,
		MP_CustomizedUVs3,
		MP_CustomizedUVs4,
		MP_CustomizedUVs5,
		MP_CustomizedUVs6,
		MP_CustomizedUVs7,
		MP_Max,
	};

	ENGINE_API EMaterialProperty stringToMaterialProperty(TCHAR* str);

	namespace EMaterialQualityLevel
	{
		enum Type
		{
			Low,
			High,
			Medium,
			Num
		};
	}

	enum EShadowMapInteractionType
	{
		SMIT_None = 0,
		SMIT_Texture = 2,
		SMIT_NumBits = 3 
	};


	enum ELightComponentType
	{
		LightType_Directional = 0,
		LightType_Point,
		LightType_Spot,
		LightType_Max,
		LightType_NumBits = 2
	};
	
}