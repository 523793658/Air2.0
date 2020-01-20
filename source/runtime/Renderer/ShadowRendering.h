#pragma once
#include "ShaderParameterUtils.h"
#include "ShaderParameters.h"
#include "Containers/DynamicRHIResourceArray.h"
#include "ScenePrivate.h"
#include "ShaderParameterMacros.h"
#include "LightSceneInfo.h"
#include "Rendering/SceneRenderTargetParameters.h"
namespace Air
{
	float getLightFadeFactor(const SceneView& view, const LightSceneProxy* proxy);

	BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(DeferredLightConstantStruct, )
		SHADER_PARAMETER(float4, ShadowMapChannelMask)
		SHADER_PARAMETER(float2, DistanceFadeMAD)
		SHADER_PARAMETER(float, ContactShadowLength)
		SHADER_PARAMETER(float, VolumetricScatteringIntensity)
		SHADER_PARAMETER(uint32, ShadowedBits)
		SHADER_PARAMETER(uint32, LightingChannelMask)
		SHADER_PARAMETER_STRUCT_INCLUDE(LightShaderParameters, LightParameters)
	END_GLOBAL_SHADER_PARAMETER_STRUCT()


	namespace StencilingGeometry
	{
		extern void drawSphere(RHICommandList& RHICmdList);

		extern void drawVectorSphere(RHICommandList& RHICmdList);

		extern void drawCone(RHICommandList& RHICmdList);


		template<int32 NumSphereSides, int32 NumSphereRings, typename VectorType>
		class TStencilSphereVertexBuffer : public VertexBuffer
		{
		public:
			int32 getNumRings() const
			{
				return NumSphereRings;
			}

			void initRHI() override
			{
				const int32 numSides = NumSphereSides;
				const int32 numRings = NumSphereRings;
				const int32 numVerts = (numSides + 1) * (numRings + 1);
				const float radiansPerRingSegment = PI / (float)numRings;
				float radius = 1;

				TArray<VectorType, TInlineAllocator<numRings + 1>> arcVerts;
				arcVerts.empty(numRings + 1);
				for (int32 i = 0; i < numRings +1; i++)
				{
					const float angle = i * radiansPerRingSegment;
					arcVerts.add(float3(0.0f, Math::sin(angle), Math::cos(angle)));
				}
				TResourceArray<VectorType, VERTEXBUFFER_ALIGNMENT> verts;
				verts.empty(numVerts);
				const float3 center = float3(0, 0, 0);
				for (int32 s = 0; s < numSides + 1; s++)
				{
					Rotator arcRotator(0, 360.f * ((float)s / numSides), 0);
					RotationMatrix arcRot(arcRotator);
					for (int32 v = 0; v < numRings + 1; v++)
					{
						const int32 vix = (numRings + 1) * s + v;
						verts.add(center + radius * float3(arcRot.transformPosition(arcVerts[v])));
					}
				}
				mNumSphereVerts = verts.size();
				uint32 size = verts.getResourceDataSize();
				RHIResourceCreateInfo createInfo(&verts);
				mVertexBufferRHI = RHICreateVertexBuffer(size, BUF_Static, createInfo);
			}

			int32 getVertexCount() const { return mNumSphereVerts;}

			void calcTransform(float4& outPosAndScale, const Sphere& sphere, const float3&  preViewTranslation, bool bConservativelyBoundShpere = true)
			{
				float radius = sphere.W;
				if (bConservativelyBoundShpere)
				{
					const int32 numRings = mNumSphereVerts;
					const float radiansPerRingSegment = PI / (float)numRings;
					radius /= Math::cos(radiansPerRingSegment);
				}
				const float3 translate(sphere.mCenter + preViewTranslation);
				outPosAndScale = float4(translate, radius);
			}

		private:
			int32 mNumSphereVerts;
		};


		template<int32 NumSphereSides, int32 NumSphereRings>
		class TStencilSphereIndexBuffer : public IndexBuffer
		{
			void initRHI() override
			{
				const int32 numSides = NumSphereSides;
				const int32 numRings = NumSphereRings;
				TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> indices;
				for (int32 s = 0; s < numSides; s++)
				{
					const int32 a0start = (s + 0) * (numRings + 1);
					const int32 a1start = (s + 1) * (numRings + 1);
					for (int32 r = 0; r < numRings; r++)
					{
						indices.add(a0start + r + 0);
						indices.add(a1start + r + 0);
						indices.add(a0start + r + 1);
						indices.add(a1start + r + 0);
						indices.add(a1start + r + 1);
						indices.add(a0start + r + 1);

					}
				}
				mNumIndices = indices.size();
				const uint32 size = indices.getResourceDataSize();
				const uint32 stride = sizeof(uint16);
				RHIResourceCreateInfo createInfo(&indices);
				mIndexBufferRHI = RHICreateIndexBuffer(stride, size, BUF_Static, createInfo);
			}

			int3 getIndexCount() const { return mNumIndices; }
		private:
			int32 mNumIndices;
		};

		class StencilConeIndexBuffer : public IndexBuffer
		{
		public:	 
			static const int32 numSides = 18;
			static const int32 numSlices = 12;

			static const uint32 numVerts = numSlices * numSides * 2;

			void initRHI() override
			{
				TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> indices;

				indices.empty((numSlices - 1) * numSides * 12);
				for (int32 sliceIndex = 0; sliceIndex < numSlices - 1; sliceIndex++)
				{
					for (int32 sideIndex = 0; sideIndex < numSides; sideIndex++)
					{
						const int32 currentIndex = sliceIndex * numSides + sideIndex % numSides;
						const int32 nextSideIndex = sliceIndex * numSides + (sideIndex + 1) % numSides;
						const int32 nextSliceIndex = (sliceIndex + 1) * numSides + sideIndex % numSides;
						const int32 nextSliceAndSideIndex = (sliceIndex + 1) * numSides + (sideIndex + 1) % numSides;
						indices.add(currentIndex);
						indices.add(nextSideIndex);
						indices.add(nextSliceIndex);
						indices.add(nextSliceIndex);
						indices.add(nextSideIndex);
						indices.add(nextSliceAndSideIndex);
					}
				}
				const int32 capIndexStart = numSides * numSlices;
				for (int32 sliceIndex = 0; sliceIndex < numSlices - 1; sliceIndex++)
				{
					for (int32 sideIndex = 0; sideIndex < numSides; sideIndex++)
					{
						const int32 currentIndex = sliceIndex * numSides + sideIndex % numSides + capIndexStart;
						const int32 nextSideIndex = sliceIndex * numSides + (sideIndex + 1) % numSides + capIndexStart;
						const int32 nextSliceIndex = (sliceIndex + 1) * numSides + sideIndex % numSides + capIndexStart;
						const int32 nextSliceAndSideIndex = (sliceIndex + 1) * numSides + (sideIndex + 1) % numSides + capIndexStart;
						indices.add(currentIndex);
						indices.add(nextSliceIndex);
						indices.add(nextSideIndex);
						indices.add(nextSideIndex);
						indices.add(nextSliceIndex);
						indices.add(nextSliceAndSideIndex);
					}
				}

				const uint32 size = indices.getResourceDataSize();
				const uint32 stride = sizeof(uint16);
				mNumIndices = indices.size();
				RHIResourceCreateInfo createInfo(&indices);
				mIndexBufferRHI = RHICreateIndexBuffer(stride, size, BUF_Static, createInfo);
			}

			int32 getIndexCount() const { return mNumIndices; }

		protected:
			int32 mNumIndices;
		};

		class StencilConeVertexBuffer : public VertexBuffer
		{
		public:
			static const int32 numVerts = StencilConeIndexBuffer::numSides * StencilConeIndexBuffer::numSlices * 2;
			void initRHI() override
			{
				TResourceArray<float4, VERTEXBUFFER_ALIGNMENT> verts;
				verts.empty(numVerts);
				for (int32 s = 0; s < numVerts; s++)
				{
					verts.add(float4(0, 0, 0, 0));
				}
				uint32 size = verts.getResourceDataSize();
				RHIResourceCreateInfo createInfo(&verts);
				mVertexBufferRHI = RHICreateVertexBuffer(size, BUF_Static, createInfo);
			}

			int32 getVertexCount() const { return numVerts; }
		};

		extern TGlobalResource<TStencilSphereVertexBuffer<18, 12, float4>> GStencilSphereVertexBuffer;
		extern TGlobalResource<TStencilSphereVertexBuffer<18, 12, float3>> GStencilSphereVectorBuffer;

		extern TGlobalResource<TStencilSphereIndexBuffer<18, 12>> GStencilSphereIndexBuffer;
		extern TGlobalResource<TStencilSphereVertexBuffer<4, 4, float4>> GLowPolyStencilSphereVertexBuffer;
		extern TGlobalResource<TStencilSphereIndexBuffer<4, 4>> GLowPolyStencilSphereIndexBuffer;

		extern TGlobalResource<StencilConeVertexBuffer> GStencilConeVertexBuffer;
		extern TGlobalResource<StencilConeIndexBuffer> GStencilConeIndexBuffer;


	}

	class StencilingGeometryShaderParameters
	{
	public:
		void bind(const ShaderParameterMap& parameterMap)
		{
			mStencilGeometryPosAndScale.bind(parameterMap, TEXT("StencilingGeometryPosAndScale"));
			mStencilConeParameters.bind(parameterMap, TEXT("StencilingConeParameters"));
			mStencilConeTransform.bind(parameterMap, TEXT("StencilingConeTransform"));
			mStencilPreViewTranslation.bind(parameterMap, TEXT("StencilingPreViewTranslation"));
		}

		void set(RHICommandList& RHICmdList, Shader* shader, const float4& inStencilingGeometryPosAndScale) const
		{
			setShaderValue(RHICmdList, shader->getVertexShader(), mStencilGeometryPosAndScale, inStencilingGeometryPosAndScale);
			setShaderValue(RHICmdList, shader->getVertexShader(), mStencilConeParameters, float4(0, 0, 0, 0));
		}

		void set(RHICommandList& RHICmdList, Shader* shader, const SceneView& view, const LightSceneInfo* lightSceneInfo) const
		{
			float4 geometryPosAndScale;
			if (lightSceneInfo->mProxy->getLightType() == LightType_Point)
			{

			}
			else if (lightSceneInfo->mProxy->getLightType() == LightType_Spot)
			{

			}
		}

		friend Archive & operator << (Archive& ar, StencilingGeometryShaderParameters& p)
		{
			ar << p.mStencilGeometryPosAndScale;
			ar << p.mStencilConeParameters;
			ar << p.mStencilConeTransform;
			ar << p.mStencilPreViewTranslation;
		}

	private:
		ShaderParameter mStencilGeometryPosAndScale;
		ShaderParameter mStencilConeParameters;
		ShaderParameter mStencilConeTransform;
		ShaderParameter mStencilPreViewTranslation;
	};

	template<typename ShaderRHIParamRef>
	void setDeferredLightParameters(RHICommandList& RHICmdList, const ShaderRHIParamRef shaderRHI,
		const TShaderConstantBufferParameter<DeferredLightConstantStruct>& deferredLightConstantBufferParameter, const LightSceneInfo* lightSceneInfo, const SceneView& view)
	{

		DeferredLightConstantStruct deferredLightConstantsValue;
		lightSceneInfo->mProxy->getLightShaderParameters(
			deferredLightConstantsValue.LightParameters
		);

		

		const float2 fadParams = lightSceneInfo->mProxy->getDirectionalLightDistanceFadeParameters(view.getFeatureLevel(), false);
		deferredLightConstantsValue.DistanceFadeMAD = float2(fadParams.y, -fadParams.x * fadParams.y);

		int32 shadowMapChannel = lightSceneInfo->mProxy->getShadowMapChannel();

		const bool bAllowStaticLighting = false;
		if (!bAllowStaticLighting)
		{
			shadowMapChannel = INDEX_NONE;
		}

		deferredLightConstantsValue.ShadowMapChannelMask = float4(
			shadowMapChannel == 0 ? 1 : 0,
			shadowMapChannel == 1 ? 1 : 0,
			shadowMapChannel == 2 ? 1 : 0,
			shadowMapChannel == 3 ? 1 : 0);

		const bool bDynamicShadows = view.mFamily->mEngineShowFlags.DynamicShadows && getShadowQuality() > 0;
		const bool bHasLightFunction = lightSceneInfo->mProxy->getLightFunctionMaterial() != nullptr;
		deferredLightConstantsValue.ShadowedBits = lightSceneInfo->mProxy->castsStaticShadow() || bHasLightFunction ? 1 : 0;
		deferredLightConstantsValue.ShadowedBits |= lightSceneInfo->mProxy->castsDynamicShadow() && view.mFamily->mEngineShowFlags.DynamicShadows ? 3 : 0;


		const ELightComponentType lightType = (ELightComponentType)lightSceneInfo->mProxy->getLightType();
		if ((lightType == LightType_Point || lightType == LightType_Spot) && view.isPerspectiveProjection())
		{
			deferredLightConstantsValue.LightParameters.Color *= getLightFadeFactor(view, lightSceneInfo->mProxy);
		}
		deferredLightConstantsValue.LightingChannelMask = lightSceneInfo->mProxy->getLightingChannelMask();

		setConstantBufferParameterImmediate(RHICmdList, shaderRHI, deferredLightConstantBufferParameter, deferredLightConstantsValue);

	}

	BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(ShadowDepthPassConstantParameters,)
		SHADER_PARAMETER_STRUCT(SceneTextureConstantParameters, SceneTextures)
		SHADER_PARAMETER(Matrix, ProjectionMatrix)
		SHADER_PARAMETER(Matrix, ViewMatrix)
		SHADER_PARAMETER(float4, ShadowParams)
		SHADER_PARAMETER(float, bClampToNearPlane)
		SHADER_PARAMETER_ARRAY(Matrix, ShadowViewProjectioinMatrices, [6])
		SHADER_PARAMETER_ARRAY(Matrix, ShadowViewMatrices, [6])
	END_GLOBAL_SHADER_PARAMETER_STRUCT()

	class ShadowMapRenderTargets
	{
	public:
		TArray<IPooledRenderTarget*, SceneRenderingAllocator> mColorTargets;
		IPooledRenderTarget* mDepthTarget;

		ShadowMapRenderTargets()
			:mDepthTarget(nullptr)
		{}

		int2 getSize() const
		{
			if (mDepthTarget)
			{
				return mDepthTarget->getDesc().mExtent;
			}
			else
			{
				BOOST_ASSERT(mColorTargets.size() > 0);
				return mColorTargets[0]->getDesc().mExtent;
			}
		}
	};

	enum EShadowDepthCacheMode
	{
		SDCM_MovablePrimitivesOnly,
		SDCM_StaticPrimitivesOnly,
		SDCM_Uncached,
	};

	class ProjectedShadowInfo : public RefCountedObject
	{
	public:
		typedef TArray<const PrimitiveSceneInfo*, SceneRenderingAllocator> PrimitiveArrayType;

		ViewInfo* mShadowDepthView;

		TConstantBufferRef<ShadowDepthPassConstantParameters> mShadowdepthPassConstantBuffer;

		ShadowMapRenderTargets mRenderTargets;

		EShadowDepthCacheMode mCacheMode;

		ViewInfo* mDependentView;

		int32 mShadowId;

		float3 mPreShadowTranslation;

		Matrix mShadowViewMatrix;

		Matrix mSubjectAndReceiverMatrix;

		Matrix mReceiverMatrix;

		Matrix mInvReceiverMatrix;

		float mInvMaxSubjectDepth;

		float mMaxSubjectZ;
		float mMinSubjectZ;

		ConvexVolume mCasterFrustum;
		ConvexVolume mReceiverFrustum;

		float mMinPreSubjectZ;

		Sphere mShadowBounds;

		ShadowCascadeSettings mCascadeSettings;

		uint32 mX;

		uint32 mY;

		uint32 mResolutionX;

		uint32 mResolutionY;

		uint32 mBorderSize;

		float mMaxScreenPercent;

		TArray<float, TInlineAllocator<2>> mFadeAlphas;

		uint32 bAllocated : 1;

		uint32 bRendered : 1;

		uint32 bAllocatedInPreshadowCache : 1;

		uint32 bDepthsCached : 1;

		uint32 bDirectionalLight : 1;

		uint32 bOnePassPointLightShadow : 1;

		uint32 bWholeSceneShadow : 1;

		uint32 bReflectiveShadowmap : 1;

		uint32 bTranslucentShadow : 1;


	};
}