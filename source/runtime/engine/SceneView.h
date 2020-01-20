#pragma once
#include "EngineMininal.h"
#include "SceneInterface.h"
#include "ShowFlags.h"
#include "SceneTypes.h"
#include "Classes/Engine/GameViewportClient.h"
#include "SceneManagement.h"
#include "RHIDefinitions.h"
#include "ConvexVolume.h"
#include "scene.h"
#include "ConstantBuffer.h"
#include "ShaderParameters.h"
#include "DebugViewModeHelpers.h"
#include "SceneViewBuffer.h"
#include "FinalPostProcessSettings.h"
#include <set>
namespace Air
{
	class RenderTarget;
	class SceneViewStateInterface;
	class AActor;
	class SceneViewFamily;
	class ViewElementDrawer;

	enum class ERayTracingRenderMode
	{
		Disabled				= 0,
		Pathtracing				= 1,
		RayTracingdebug			= 2,
	};


	static const Matrix invertProjectionMatrix(const Matrix& M)
	{
		if (M.M[1][0] == 0.0f &&
			M.M[3][0] == 0.0f &&
			M.M[0][1] == 0.0f &&
			M.M[3][1] == 0.0f &&
			M.M[0][2] == 0.0f &&
			M.M[1][2] == 0.0f &&
			M.M[0][3] == 0.0f &&
			M.M[1][3] == 0.0f &&
			M.M[2][3] == 1.0f &&
			M.M[3][3] == 0.0f)
		{
			double a = M.M[0][0];
			double b = M.M[1][1];
			double c = M.M[2][2];
			double d = M.M[3][2];
			double s = M.M[2][0];
			double t = M.M[2][1];

			return Matrix(
				Plane(1.0f / a, 0.0f, 0.0f, 0.0f),
				Plane(0.0f, 1.0 / b, 0.0f, 0.0f),
				Plane(0.0f, 0.0f, 0.0f, 1.0f / d),
				Plane(-s / a, -t / b, 1.0f, -c / d)
			);
		}
		else
		{
			return M.inverse();
		}
	}



	enum ETranslucencyValumeCascade
	{
		TVC_Inner,
		TVC_Outer,
		TVC_Max
	};


	struct SceneViewProjectionData 
	{
		float3 mViewOrigin;
		Matrix mViewRotationMatrix;

		Matrix mProjectionMatrix;

	protected:
		IntRect mViewRect;
		IntRect mConstrainedViewRect;

	public:
		void setViewRectangle(const IntRect& inViewRect)
		{
			mViewRect = inViewRect;
			mConstrainedViewRect = inViewRect;
		}
		void setConstrainedViewRectangle(const IntRect& inViewRect)
		{
			mConstrainedViewRect = inViewRect;
		}

		bool isValidViewRectangle()const
		{
			return (mConstrainedViewRect.min.x >= 0) &&
				(mConstrainedViewRect.min.y >= 0) &&
				(mConstrainedViewRect.width() > 0) &&
				(mConstrainedViewRect.height() > 0);
		}

		bool isPerspectiveProjection() const
		{
			return mProjectionMatrix.M[3][3] < 1.0f;
		}

		const IntRect& getViewRect() const { return mViewRect; }

		const IntRect& getConstrainedViewRect() const { return mConstrainedViewRect; }

		Matrix computeViewProjectionMatrix() const
		{
			return TranslationMatrix(-mViewOrigin) * mViewRotationMatrix * mProjectionMatrix;
		}
	};


	struct SceneViewInitOptions : public SceneViewProjectionData 
	{
		const SceneViewFamily* mViewFamily;
		SceneViewStateInterface* mSceneViewStateInterface;

		std::shared_ptr<AActor> mViewActor;
		ViewElementDrawer* mViewElementDrawer;

		LinearColor mBackgroundColor;
		LinearColor mOverlayColor;
		LinearColor mColorScale;

		EStereoscopicPass mStereoPass;

		float mWorldToMetersScale;

		std::set<PrimitiveComponentId> mHiddenPrimitives;

		std::set<PrimitiveComponentId> mShowOnlyPrimitives;

		int2	mCursorPos;

		float mLodDistanceFactor;

		float OverrideFarClippingPlaneDistance;

		float3 mOriginOffsetThisFrame;
		bool mInCameraCut;

		bool mUseFieldOfViewForLOD;

		
		SceneViewInitOptions()
			:mViewFamily(nullptr)
			, mSceneViewStateInterface(nullptr)
			, mViewActor(nullptr)
			, mViewElementDrawer(nullptr)
			, mBackgroundColor(LinearColor::Transparent)
			, mOverlayColor(LinearColor::Transparent)
			, mColorScale(LinearColor::White)
			, mStereoPass(eSSP_FULL)
			, mWorldToMetersScale(100.0f)
			, mCursorPos(-1, -1)
			, mLodDistanceFactor(1.0f)
			, OverrideFarClippingPlaneDistance(-1.0f)
			, mOriginOffsetThisFrame(ForceInitToZero)
			, mInCameraCut(false)
			, mUseFieldOfViewForLOD(true)
		{
		}
	};

	struct ViewMatrices
	{
		ViewMatrices()
		{
			mProjectionMatrix.setIdentity();
			mViewMatrix.setIdentity();
			mHMDViewMatrixNoRoll.setIdentity();
			mTranslatedViewMatrix.setIdentity();
			mTranslatedViewProjectionMatrix.setIdentity();
			mInvTranslatedViewProjectionMatrix.setIdentity();
			mPreviewTranslation = float3::Zero;
			mViewOrigin = float3::Zero;
			mProjectionScale = float2::zero();
			mScreenScale = 1.0f;
		}


		ViewMatrices(const SceneViewInitOptions& inOptions);

		inline const float3 & getViewOrigin() const
		{
			return mViewOrigin;
		}

		inline const Matrix& getViewMatrix() const
		{
			return mViewMatrix;
		}

		inline const Matrix& getViewProjectionMatrix() const
		{
			return mViewProjectionMatrix;
		}

		inline const Matrix& getProjectionMatrix() const
		{
			return mProjectionMatrix;
		}

		inline const Matrix& getInvProjectionMatrix() const
		{
			return mInvProjectionMatrix;
		}

		inline bool isPerspectiveProjection() const
		{
			return mProjectionMatrix.M[3][3] < 1.0f;
		}

		inline const float3& getPreviewTranslation() const
		{
			return mPreviewTranslation;
		}

		inline const Matrix& getInvViewMatrix() const
		{
			return mInvViewMatrix;
		}

		inline const Matrix& getTranslatedViewMatrix() const
		{
			return mTranslatedViewMatrix;
		}

		inline const Matrix& getInvTranslatedViewMatrix() const
		{
			return mInvTranslatedViewMatrix;
		}

		inline const Matrix& getInvTranslatedViewProjectionMatrix() const
		{
			return mInvTranslatedViewProjectionMatrix;
		}

		inline const Matrix& getTranslatedViewProjectionMatrix() const
		{
			return mTranslatedViewProjectionMatrix;
		}

		inline const Matrix& getOverriddenTranlatedViewMatrix() const
		{
			return mOverriddenTranslatedViewMatrix;
		}

		inline const Matrix& getOverriddenInvTranslatedViewMatrix() const
		{
			return mOverriddenInvTranslatedViewMatrix;
		}

		inline const Matrix& getInvViewProjectionMatrix() const
		{
			return mInvViewProjectionMatrix;
		}
	private:
		Matrix	mProjectionMatrix;
		Matrix	mInvProjectionMatrix;
		
		Matrix	mViewMatrix;
		Matrix	mInvViewMatrix;
		
		Matrix	mViewProjectionMatrix;
		Matrix	mInvViewProjectionMatrix;

		Matrix	mHMDViewMatrixNoRoll;

		Matrix	mTranslatedViewMatrix;
		Matrix	mInvTranslatedViewMatrix;

		Matrix	mOverriddenTranslatedViewMatrix;
		Matrix	mOverriddenInvTranslatedViewMatrix;

		Matrix	mTranslatedViewProjectionMatrix;
		Matrix	mInvTranslatedViewProjectionMatrix;

		float3	mPreviewTranslation;
		float3	mViewOrigin;
		float2	mProjectionScale;
		
		float	mScreenScale;



	};

	namespace EDrawDynamicFlags
	{
		enum Type
		{
			None = 0,
			ForceLowestLOD = 0x1
		};
	}

	BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT_WITH_CONSTRUCTOR(ViewConstantShaderParameters, ENGINE_API)
		VIEW_CONSTANT_BUFFER_MEMBER_TABLE
		SHADER_PARAMETER_TEXTURE(TextureCube, SkyBoxTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, SkyBoxTextureSampler)
		SHADER_PARAMETER_SRV(StructuredBuffer<float4>, PrimitiveSceneData)
	END_GLOBAL_SHADER_PARAMETER_STRUCT(ViewConstantShaderParameters)

	class ForwardLightingViewResources;

	class ENGINE_API SceneView
	{
	public:
		SceneView(const SceneViewInitOptions& initOptions);

		void startFinalPostprocessSettings(float3 inViewLocation);


		void setupCommonViewConstantBufferParameters(ViewConstantShaderParameters& viewConstantShaderParameters, const int2& bufferSize, int32 numMSAASamplers, const IntRect& effectiveViewRect, const ViewMatrices& inViewMatrices, const ViewMatrices& inPreViewMatrices) const;

		void setupViewRectConstantBufferParameters(ViewConstantShaderParameters& viewConstantShaderParameters, const int2& inBufferSize, const IntRect& inEffectiveViewRect, const ViewMatrices& inViewMatrices, const ViewMatrices& inPrevViewMatrice) const;

		inline float3 getViewDirection() const { return mViewMatrices.getViewMatrix().getColomn(2); }

		bool isInstancedStereoPass() const { return bIsInstancedStereoEnabled && mStereoPass == eSSP_LEFT_EYE; }

		ERHIFeatureLevel::Type getFeatureLevel() const { return mFeatureLevel; }

		inline bool isPerspectiveProjection() const { return mViewMatrices.isPerspectiveProjection(); }

		EShaderPlatform getShaderPlatform() const;


	public:

		const SceneViewFamily* mFamily;
		TConstantBufferRef<ViewConstantShaderParameters> mViewConstantBuffer;

		TConstantBufferRef<ViewConstantShaderParameters> mDownsampledTranslucencyViewConstantBuffer;
		
		ForwardLightingViewResources* mForwardLightingResources;

		IntRect mCameraConstrainedViewRect;

		const IntRect mUnscaledViewRect;

		float3 mViewLocation;
		Rotator mViewRotation;
		Quaternion mBaseHmdOrientation;
		float3 mBaseHmdLocation;
		float mWorldToMetersScale;

		SceneViewStateInterface* mState;

		ViewElementDrawer* mDrawer;

		std::shared_ptr<AActor> mViewActor;

		IntRect mViewRect;

		IntRect mUnconstrainedViewRect;

		int32 mMaxShadowCascades{ 10 };

		ViewMatrices mViewMatrices;
		ViewMatrices mPrevViewMatrices;

		ViewMatrices mShadowViewMatrices;

		Matrix mProjectionMatrixUnadjustedForRHI;

		float4 mInvDeviceZToWorldZTransform;

		LinearColor mBackgroundColor;
		LinearColor mOverlayColor;

		LinearColor mColorScale;

		EStereoscopicPass mStereoPass;
		EAntiAliasingMethod mAntiAliasingMethod;
		FinalPostProcessSettings mFinalPostProcessSettings;

		bool bRenderFirstInstanceOnly{ false };


		float4 mDiffuseOverrideParameter{ float4(0, 0, 0, 1) };
		float4 mSpecularOverrideParameter{ float4(0, 0, 0, 1) };
		float4 mNormalOverrideParameter{ float4(0, 0, 0, 1) };
		float2 mRoughnessOverrideParameter{ float2(0, 1) };

		bool bIsGameView{ false };
		bool bIsViewInfo{ false };
		bool bIsSeneCapture{ false };
		bool bIsRefectionCapture{ false };
		bool bIsPlanarRefelction{ false };
		bool bRenderSceneTwoSided{ false };
		bool bIsLocked{ false };
		bool bStaticSceneOnly{ false };
		bool bIsInstancedStereoEnabled{ false };
		bool bIsMuliViewEnabled{ false };
		bool bIsMobileMultiViewEnable{ false };
		bool bShouldBindInstancedViewUB{ false };
		bool bHasNearClippingPlane{ true };
		bool bReverseCulling;
		bool bUseFieldOfViewForLOD{ false };
		bool bAllowTemporalJitter;
		float mTemporalJitterPixelsX;
		float mTemporalJitterPixelsY;
		Plane mGlobalClippingPlane{ Plane(0, 0, 0, 0) };
		ERHIFeatureLevel::Type mFeatureLevel;

		ConvexVolume mViewFrustum;
		Plane mNearClippingPlane;

		float mNearClippingDistance{ 0 };

		EDrawDynamicFlags::Type mDrawDynamicFlags;

		TextureRHIRef mSkyTexture;
	};

	struct SceneViewScreenPercentageConfig
	{
		float mPrimaryResolutionFraction;

		SceneViewScreenPercentageConfig()
			:mPrimaryResolutionFraction(1.0f)
		{}

		static constexpr float kMinResolutionFraction = 0.01f;
		static constexpr float kMaxResolutionFraction = 4.0f;

		static constexpr float kMinTAAUpsampleResolutionFraction = 0.5f;
		static constexpr float kMaxTAAUpsampleResolutionFraction = 2.0f;

#if DO_CHECK
		static bool isValidResolutionFraction(float resolutionFraction)
		{
			return resolutionFraction >= kMinResolutionFraction && resolutionFraction <= kMaxResolutionFraction;
		}
#endif
	};

	class ENGINE_API ISceneViewFamilyScreenPercentage
	{
	protected:
		virtual ~ISceneViewFamilyScreenPercentage() {};

		virtual float getPrimaryResolutionFractionUpperBound() const = 0;

		virtual ISceneViewFamilyScreenPercentage* fork_GameThread(const class SceneViewFamily& viewFamily) const = 0;

		virtual void computePrimaryResolutionFractions_RenderThread(TArray<SceneViewScreenPercentageConfig>& outViewScreenPercentageConfigs) const = 0;

		friend class SceneViewFamily;
		friend class SceneRenderer;
	};

	enum class ESecondaryScreenPercentageMethod
	{
		NearestSpatialUpscale,
		LowerPixelDensitySimulation,
	};

	class ENGINE_API SceneViewFamily
	{
	public:
		struct ConstructionValues
		{
			ConstructionValues(const RenderTarget* inRenderTarget, SceneInterface* inScene, const EngineShowFlags& inEngineShowFlags)
				: mRenderTarget(inRenderTarget),
				mScene(inScene),
				mEngineShowFlags(inEngineShowFlags),
				mViewModeParam(-1),
				mCurrentWorldTime(0.0f),
				mDeltaWorldTime(0.0f),
				mCurrentRealTime(0.0f),
				mGammaCorrection(1.0f),
				mMonoFarFieldCullingDistance(0.0f),
				bRealtimeUpdate(false),
				bDeferClear(false),
				bResolveScene(true),
				bTimesSet(false)
			{
				
			}

			const RenderTarget* mRenderTarget{ nullptr };

			SceneInterface* mScene{ nullptr };

			EngineShowFlags mEngineShowFlags;

			int32 mViewModeParam;

			wstring mViewModeParamName;

			float mCurrentWorldTime;

			float mDeltaWorldTime;

			float mCurrentRealTime;

			float mGammaCorrection;

			float mMonoFarFieldCullingDistance;

			bool bRealtimeUpdate : 1;
			
			bool bDeferClear : 1;

			bool bResolveScene : 1;

			bool bTimesSet : 1;

			ConstructionValues& setWorldTime(const float inCurrentWorldTime, const float inDeltaWorldTime, const float inCurrentRealTime)
			{
				mCurrentRealTime = inCurrentRealTime;
				mDeltaWorldTime = inDeltaWorldTime;
				mCurrentWorldTime = inCurrentWorldTime;
				bTimesSet = true;
				return *this;
			}

			ConstructionValues& setRealtimeUpdates(const bool value)
			{
				bRealtimeUpdate = value;
				return *this;
			}

			ConstructionValues& setDeferClear(const bool value)
			{
				bDeferClear = value;
				return *this;
			}

			ConstructionValues& setResolveScene(const bool value)
			{
				bResolveScene = value;
				return *this;
			}
			ConstructionValues& setGammaCorrection(const float value)
			{
				mGammaCorrection = value;
				return *this;
			}

			ConstructionValues& setViewModeParma(const int inViewModeParam, const wstring & inViewModelParamName)
			{
				mViewModeParam = inViewModeParam;
				mViewModeParamName = inViewModelParamName;
				return *this;
			}

		};
	public:


		SceneViewFamily(const ConstructionValues& CVS);

		ERHIFeatureLevel::Type getFeatureLevel() const
		{
			if (mScene)
			{
				return mScene->getFeatureLevel();
			}
			else
			{
				return GMaxRHIFeatureLevel;
			}
		}

		EShaderPlatform getShaderPlatform() const { return GShaderPlatformForFeatureLevel[getFeatureLevel()]; }

		void computeFamilySize();
#if !(BUILD_SHIPPING || BUILD_TEST)
		EDebugViewShaderMode mDebugViewShaderMode;

		FORCEINLINE bool useDebugViewVSDSHS()const { return false; }
		FORCEINLINE bool useDebugViewPS() const { return false; }
		FORCEINLINE EDebugViewShaderMode getDebugViewShaderMode() const {
			return mDebugViewShaderMode;
		}
#endif

		const SceneView& getSteredEyeView(const EStereoscopicPass eye) const;

		bool supportsScreenPercentage() const;

		FORCEINLINE float getPrimaryResolutionFractionUpperBound() const
		{
			BOOST_ASSERT(mScreenPercentageInterface != nullptr);
			float primaryUpperBoundFraction = mScreenPercentageInterface->getPrimaryResolutionFractionUpperBound();

			if (!mEngineShowFlags.ScreenPercentage)
			{
				BOOST_ASSERT(primaryUpperBoundFraction >= 1.0f);
			}

			return primaryUpperBoundFraction;
		}
	public:
		SceneInterface* mScene{ nullptr };

		TArray<const SceneView*> mViews;

		uint32 mFamilySizeX{ 0 };
		uint32 mFamilySizeY{ 0 };
		uint32 mInstancedStereoWidth{ 0 };
		const RenderTarget* mRenderTarget{ nullptr };
		bool bUseSepareteRenderTarget{ false };
		EngineShowFlags mEngineShowFlags;
		float mCurrentWorldTime;
		float mDeltaWorldTime;
		float mCurrentRealTime;
		uint32 mFrameNumber;
		bool bRealtimeUpdate;
		bool bDeferClear;
		bool bResolveScene;
		bool bEarlyZPassMovable;

		float mSecondaryViewFraction;
		ESecondaryScreenPercentageMethod mSecondaryScreenPercentageMethod;

		ESceneCaptureSource mSceneCaptureSource{ SCS_FinalColorLDR };

		ESceneCaptureCompositeMode mSceneCaptureCompositeMode{ SCCM_Overrite };
		bool bWorldIsPaused{ false };
		float mGammaCorrect;

		TArray<std::shared_ptr<class ISceneViewExtension>> mViewExtensions;

	private:
		SceneViewFamily(const SceneViewFamily&) = default;

		ISceneViewFamilyScreenPercentage* mScreenPercentageInterface;

		friend class SceneRenderer;
	};

	class SceneViewFamilyContext : public SceneViewFamily
	{
	public:
		SceneViewFamilyContext(const ConstructionValues& CVS)
			: SceneViewFamily(CVS)
		{}

		ENGINE_API ~SceneViewFamilyContext();
	};

	

	
}