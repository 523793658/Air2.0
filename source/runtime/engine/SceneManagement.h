#pragma once
#include "EngineMininal.h"
#include "RHI.h"
#include "RHIResource.h"
#include "SceneTypes.h"
#include "SceneUtils.h"
#include "Math/Sphere.h"
#include "RHICommandList.h"
#include "RenderResource.h"
#include "EngineDefines.h"
#include "Classes/Components/PrimitiveComponent.h"
#include "Misc/Guid.h"
namespace Air
{
	class LightComponent;
	class LightMap;
	class MaterialRenderProxy;
	static const int32 MAX_NUM_LIGHTMAP_COEF = 2;


	class ENGINE_API LightSceneProxy
	{
	public:
		LightSceneProxy(const LightComponent* inLightComponent);

		virtual ~LightSceneProxy()
		{

		}

		inline const LinearColor getColor() const
		{
			return mColor;
		}

		inline const float4 getPosition() const
		{
			return mPosition;
		}

		inline const float3 getDirection() const
		{
			return float3(mWorldToLight.M[0][2], mWorldToLight.M[1][2], mWorldToLight.M[2][2]);
		}

		void setTransform(const Matrix& inLightToWorld, const float4& inPosition);

		inline bool hasStaticLighting()const { return bStaticLighting; }

		inline const MaterialRenderProxy* getLightFunctionMaterial() const { return mLightFunctionMaterial; }

		virtual void getParameters(float4& lightPositionAndInvRadius, float4& lightColorAndFalloffExponent, float3& normalizedLightDirection, float2& spotAngles, float& lightSourceRadius, float& lightSourceLength, float& lightMinRoughness) const {}

		virtual float2 getDirectionalLightDistanceFadeParameters(ERHIFeatureLevel::Type inFealtureLevel, bool bPrecomputedLightIsValid) const
		{
			return float2(0, 0);
		}

		inline int32 getShadowMapChannel() const { return mShadowMapChannel; }

		inline uint8 getLightType() const { return mLightType; }

		inline bool castsStaticShadow() const 
		{ 
			return bCastStaticShadow;
		}
		inline bool castsDynamicShadow() const 
		{
			return bCastDynamicShadow;
		}

		inline bool hasStaticShadowing() const { return bStaticShadowing; }

		inline const LightComponent* getLightComponent() const { return mLightComponent; }

		virtual bool isInverseSquared() const 
		{
			return false;
		}

		virtual Sphere getBoundingSphere() const
		{
			return Sphere(getPosition(), Math::min(getRadius(), (float)WORLD_MAX));
		}


		virtual float getMaxDrawDistance() const { return 0.0f; }

		virtual float getFadeRange() const { return 0.0f; }

		inline uint8 getLightingChannelMask() const { return mLightingChannelMask; }

		virtual float getRadius() const { return FLT_MAX; }

		inline float3 getOrigin() const { return mLightToWorld.getOrigin(); }

		inline bool isUsedAsAtmosphereSunLight() const {
			return bUsedAsAtmosphereSunLight;
		}

		inline class LightSceneInfo* getLightSceneInfo() const {
			return mLightSceneInfo;
		}

		void setColor(const LinearColor& inColor);


	protected:
		friend class Scene;
		class LightSceneInfo* mLightSceneInfo;


		LinearColor mColor;
		float4 mPosition;
		Matrix mWorldToLight;
		Matrix mLightToWorld;
		const LightComponent* mLightComponent{ nullptr };
		const uint8 mLightType;

		const uint32 bStaticLighting : 1;

		const uint32 bStaticShadowing : 1;

		const uint32 bCastStaticShadow : 1;
		
		const uint32 bCastDynamicShadow : 1;

		const uint32 bUsedAsAtmosphereSunLight : 1;

		const uint32 bMovable : 1;

		int32 mShadowMapChannel;

		uint8 mLightingChannelMask;

		float mMinRoughness{ 0.04f };

		float mIndirectLightingScale;

		const MaterialRenderProxy* mLightFunctionMaterial;


		Guid mLightGuid;
	};

	class SceneViewStateInterface
	{
	public:
		SceneViewStateInterface()
			:mViewParent(nullptr),
			mNumChildren(0)
		{}

		bool isViewParent() const
		{
			return mNumChildren > 0;
		}

		virtual void destroy() = 0;

	private:
		SceneViewStateInterface* mViewParent;
		int32					mNumChildren;
	};

	class ENGINE_API ViewElementDrawer
	{

	};

	class DynamicPrimitiveResource
	{
	public:
		virtual void initPrimitiveResource() = 0;
		virtual void releasePrimitiveResource() = 0;
	};
	struct MeshBatch;
	class PrimitiveSceneProxy;

	struct MeshBatchAndRelevance
	{

		const MeshBatch* Mesh;
		const PrimitiveSceneProxy* mPrimitiveSceneProxy;
	private:

		uint32 bHashOpaqueOrMaskedMaterial : 1;
		uint32 bRenderInMainPass : 1;
	public:
		MeshBatchAndRelevance(const MeshBatch* inMesh, const PrimitiveSceneProxy* inPrimitiveSceneProxy, ERHIFeatureLevel::Type featureLevel);
		bool getHasOpaqueOrMaskedMaterial() const { return bHashOpaqueOrMaskedMaterial; }
		bool getRenderInMainPass() const { return bRenderInMainPass; }
	};


	class LightMapInteraction
	{
	public:
		static LightMapInteraction none()
		{
			LightMapInteraction result;
			result.mType = LMIT_None;
			return result;
		}

		LightMapInteraction()
			:mType(LMIT_None)
		{}
		ELightMapInteractionType getType() const { return mType; }

		void setLightMapInteractionType(ELightMapInteractionType inType)
		{
			mType = inType;
		}
	private:
		ELightMapInteractionType mType;

	};


	class LightCacheInterface
	{
	public:
		ConstantBufferRHIParamRef getPrecomputedLightingBuffer()const
		{
			return mPrecomputedLightingConstantBuffer;
		}

	private:
		const LightMap* mLightMap;

		ConstantBufferRHIRef mPrecomputedLightingConstantBuffer;

	};

	FORCEINLINE void beginMeshDrawEvent(RHICommandList& rhiCmdList, const class PrimitiveSceneProxy* primitiveSceneProxy, const struct MeshBatch& mesh, struct TDrawEvent<RHICommandList>& drawEvent)
	{
#if WANTS_DRAW_MESH_EVENTS

#endif
	}

	class SharedSamplerState : public RenderResource
	{
	public:
		SamplerStateRHIRef mSamplerStateRHI;
		bool bWrap;
		SharedSamplerState(bool bInWrap)
			:bWrap(bInWrap)
		{}

		virtual void initRHI() override;

		virtual void releaseRHI() override
		{
			mSamplerStateRHI.safeRelease();
		}

	};

	extern ENGINE_API SharedSamplerState* Wrap_WorldGroupSettings;

	extern ENGINE_API SharedSamplerState* Clamp_WorldGroupSettings;

	extern ENGINE_API void initializeSharedSamplerStates();

	float ENGINE_API computeBoundsScreenSize(const float4& boundsOrigin, const float sphereRadius, const float4& viewOrigin, const Matrix&projMatrix);

	class StaticPrimitiveDrawInterface
	{
	public:
		virtual void drawMesh(const MeshBatch& mesh, float screenSize) = 0;
	};

	class MeshElementCollector
	{

	};
}