#pragma once
#include "EngineMininal.h"
#include "Containers/EnumAsByte.h"
namespace Air
{
	enum ESceneCaptureSource
	{
		SCS_SceneColorHDR,
		SCS_SceneColorHDRNoAlpha,
		SCS_FinalColorLDR,
		SCS_SceneColorSceneDepth,
		SCS_SceneDepth ,
		SCS_Normal,
		SCS_BaseColor
	};

	enum ESceneDepthPriorityGroup
	{
		SDPG_World,
		SDPG_Foreground,
		SDPG_MAX,
	};

	enum ESceneCaptureCompositeMode
	{
		SCCM_Overrite,
		SCCM_Additive,
		SCCM_Composite
	};

	namespace EWorldType
	{
		enum Type
		{
			None,
			Game,
			Editor,
			Demo,
		};
	}

	enum EAspectRatioAxisConstraint
	{
		AspectRatio_MaintainYFOV,
		AspectRatio_MaintainXFOV,
		AspectRatio_MajorAxisFOV,
		AspectRatio_MAX,
	};

	namespace EBrowseReturnVal
	{
		enum Type
		{
			Success,
			Failure,
			Pending,
		};
	}
	namespace EGBufferFormat
	{
		enum Type
		{
			Force8BitsPerChannel = 0,
			HighPrecisionNormals = 3,
			Force16BitsPerChannel = 5,
		};
	}

	enum EBlendMode
	{
		BLEND_Opaque,
		BLEND_Masked,
		BLEND_Translucent,
		BLEND_Additive,
		BLEND_Modulate,
		BLEND_AlphaComposite,
		BLEND_MAX
	};

	enum EMaterialShadingModel
	{
		MSM_Unlit,
		MSM_DefaultLit,
		MSM_Subsurface,
		MSM_PreintegratedSkin,
		MSM_ClearCoat,
		MSM_SubsurfaceProfile,
		MSM_TwoSidedFoliage,
		MSM_Hair,
		MSM_Cloth,
		MSM_Eye,
		MSM_MAX
	};

	enum EMaterialTessellationMode
	{
		 MTM_NOTessellation,
		 MTM_FlatTessellation,
		 MTM_PNTriangles,
		 MTM_MAX
	};

	enum ESamplerSourceMode
	{
		SSM_FromTextureAsset,
		SSM_Wrap_WorldGroupSettings,
		SSM_Clamp_WorldGroupSettings
	};

	enum ENetRole
	{
		ROLE_None,
		ROLE_SimulatedProxy,
		ROLE_AutonomousProxy,
		ROLE_Authority,
		ROLE_MAX,
	};

	enum class ETeleportType
	{
		None,
		TeleportPhysics
	};

	enum { NumInlinedActorComponents = 24 };

	struct ENGINE_API RotationConversionCache
	{
	private:
		mutable Quaternion	mCachedQuat;
		mutable Rotator mCachedRotator;
	public:

		RotationConversionCache()
			:mCachedQuat(Quaternion::identity)
			,mCachedRotator(Rotator::ZeroRotator)
		{

		}

		FORCEINLINE_DEBUGGABLE Rotator QuatToRotator(const Quaternion& inQuat) const
		{
			if (mCachedQuat != inQuat)
			{
				mCachedQuat = inQuat.getNormalized();
				mCachedRotator = mCachedQuat.rotator();
			}
			return mCachedRotator;
		}

		FORCEINLINE_DEBUGGABLE Rotator quatToRotato_readOnly(const Quaternion& inQuat) const
		{
			if (mCachedQuat == inQuat)
			{
				return mCachedRotator;
			}
			return inQuat.rotator();
		}

		FORCEINLINE_DEBUGGABLE Quaternion rotatorToQuat(const Rotator& inRotator) const
		{
			if (mCachedRotator != inRotator)
			{
				mCachedRotator = inRotator.getNormalized();
				mCachedQuat = mCachedRotator.quaternion();
			}
			return mCachedQuat;
		}

		FORCEINLINE_DEBUGGABLE Quaternion getCachedQuat() const
		{
			return mCachedQuat;
		}

		FORCEINLINE_DEBUGGABLE Quaternion rotatorToQuat_ReadOnly(const Rotator& inRotator) const
		{
			if (mCachedRotator == inRotator)
			{
				return mCachedQuat;
			}
			return inRotator.quaternion();
		}


		FORCEINLINE_DEBUGGABLE Rotator normalizedQuatToRotator(const Quaternion& inNormalizedQuat) const
		{
			if (mCachedQuat != inNormalizedQuat)
			{
				mCachedQuat = inNormalizedQuat;
				mCachedRotator = inNormalizedQuat.rotator();
			}
			return mCachedRotator;
		}
	};


	namespace ERangeBoundTypes
	{
		enum Type
		{
			Exclusive,
			Inclusive,
			Open
		};
	}

	template<typename ElementType>
	class TRangeBound
	{
	public:
		TRangeBound()
			:mType(ERangeBoundTypes::Open)
			,mValue()
		{}

		TRangeBound(const ElementType & inValue)
			:mType(ERangeBoundTypes::Inclusive)
			,mValue(inValue)
		{}

	public:
		bool operator == (TRangeBound& other) const
		{
			return ((mType == other.mType) && (isOpen() || (mValue == other.mValue)));
		}

		bool operator != (TRangeBound& other) const
		{
			return ((mType != other.mType) || (!isOpen() && (mValue != other.mValue)));
		}

		const ElementType & getValue() const
		{
			BOOST_ASSERT(mType != ERangeBoundTypes::Open);
			return mValue;
		}

		bool isClosed() const
		{
			return (mType != ERangeBoundTypes::Open);
		}


		bool isOpen() const
		{
			return (mType == ERangeBoundTypes::Open);
		}

		bool isInclusive() const
		{
			return (mType == ERangeBoundTypes::Inclusive);
		}

		bool isExclusive() const
		{
			return (mType == ERangeBoundTypes::Exclusive);
		}

	public:
		friend class Archive& operator << (class Archive& ar, TRangeBound& bound)
		{
			return ar << (uint8&)bound.mType << bound.mValue;
		}

		friend uint32 getTypeHash(const TRangeBound& bound)
		{
			return (getTypeHash((uint8)bound.mType) + 23 * getTypeHash(bound.mValue));
		}

	public:
		static FORCEINLINE TRangeBound exclusive(const ElementType& value)
		{
			TRangeBound result;
			result.mType = ERangeBoundTypes::Exclusive;
			result.mValue = value;
			return result;
		}

		static FORCEINLINE TRangeBound inclusive(const ElementType& value)
		{
			TRangeBound result;
			result.mType = ERangeBoundTypes::Inclusive;
			result.mValue = value;
			return result;
		}

		static FORCEINLINE TRangeBound open()
		{
			TRangeBound result;
			result::mType = ERangeBoundTypes::Open;
			return result;
		}

		static FORCEINLINE TRangeBound flipInclusion(const TRangeBound& bound)
		{
			if (bound.isExclusive())
			{
				return inclusive(bound.mValue);
			}
			if (bound.isInclusive())
			{
				return exclusive(bound.mValue);
			}
			return bound;
		}

		static FORCEINLINE const TRangeBound& maxLower(const TRangeBound& A, const TRangeBound& B)
		{
			if (A.isOpen()) { return B; }
			if (B.isOpen()) { return A; }
			if (A.mValue > B.mValue) { return A; }
			if (B.mValue > A.mValue) { return B; }
			if (A.isExclusive()) { return A; }
			return B;
		}

		static FORCEINLINE const TRangeBound& maxUpper(const TRangeBound& A, const TRangeBound& B)
		{
			if (A.isOpen()) { return A; }
			if (B.isOpen()) { return B; }
			if (A.mValue > B.mValue) { return A; }
			if (B.mValue > A.mValue) { return B; }
			if (A.isInclusive()) { return A; }
			return B;
		}

		static FORCEINLINE const TRangeBound& minLower(const TRangeBound& A, const TRangeBound& B)
		{
			if (A.isOpen()) { return A; }
			if (B.isOpen()) { return B; }
			if (A.mValue < B.mValue) { return A; }
			if (B.mValue < A.mValue) { return B; }
			if (A.isInclusive()) { return A; }
			return B;
		}

		static FORCEINLINE const TRangeBound& minUpper(const TRangeBound& A, const TRangeBound& B)
		{
			if (A.isOpen()) { return B; }
			if (B.isOpen()) { return A; }
			if (A.mValue < B.mValue) { return A; }
			if (B.mValue < A.mValue) { return B; }
			if (A.isExclusive()) { return A; }
			return B;
		}
	private:
		TEnumAsByte<ERangeBoundTypes::Type> mType;
		ElementType mValue;

	};

	template<typename ElementType>
	class TRange
	{
	public:
		typedef TRangeBound<ElementType> BoundsType;

		TRange() {}

		explicit TRange(const ElementType & A, const ElementType& B)
			:mLowerBound(BoundsType::inclusive(A))
			,mUpperBound(BoundsType::exclusive(B))
		{}

		bool hasLowerBound() const
		{
			return mLowerBound.isClosed();
		}

		const ElementType& getLowerBoundValue() const
		{
			return mLowerBound.getValue();
		}

		bool hasUpperBound() const
		{
			return mUpperBound.isClosed();
		}
		const ElementType& getUpperBoundValue() const
		{
			return mUpperBound.getValue();
		}
	private:
		BoundsType mLowerBound;
		BoundsType mUpperBound;
	};

	typedef TRange<float> FloatRange;

	namespace EAutoReceiveInput
	{
		enum Type
		{
			Disabled,
			Player0,
			Player1,
			Player2,
			Player3,
			Player4,
			Player5,
			Player6,
			Player7,
		};
	}

	enum class EAttachmentRule : uint8
	{
		KeepRelative,
		KeepWorld,
		SnapToTarget,
	};

	struct ENGINE_API AttachmentTransformRules
	{
		static AttachmentTransformRules keepRelativeTransform;
		static AttachmentTransformRules keepWorldTransform;
		static AttachmentTransformRules snapToTargetNotIncludingScale;
		static AttachmentTransformRules snapToTargetIncludingScale;

		AttachmentTransformRules(EAttachmentRule inRule, bool bInWellSimulatedBodies)
			:mLocationRule(inRule)
			,mRotationRule(inRule)
			,mScaleRule(inRule)
			,bWeldSimulatedBodies(bInWellSimulatedBodies)
		{}

		AttachmentTransformRules(EAttachmentRule inLocationRule, EAttachmentRule inRotationRule, EAttachmentRule inScaleRule, bool bInWeldSimulatedBodies)
			:mLocationRule(inLocationRule)
			,mRotationRule(inRotationRule)
			,mScaleRule(inScaleRule)
			, bWeldSimulatedBodies(bInWeldSimulatedBodies)
		{}



		EAttachmentRule mLocationRule;
		EAttachmentRule mRotationRule;
		EAttachmentRule mScaleRule;

		bool bWeldSimulatedBodies;
	};

	enum class EDetachmentRule : uint8
	{
		KeepRelative,
		KeepWorld
	};

	struct ENGINE_API DetachmentTransformRules
	{
		static DetachmentTransformRules keepRelativeTranform;
		static DetachmentTransformRules keepWorldTransform;

		DetachmentTransformRules(EDetachmentRule inRule, bool bInCallModify)
			:mLocationRule(inRule)
			,mRotationRule(inRule)
			,mScaleRule(inRule)
			,bCallModify(bInCallModify)
		{}

		DetachmentTransformRules(EDetachmentRule inLocationRule, EDetachmentRule inRotationRule, EDetachmentRule inScaleRule, bool bInCallModify)
			:mLocationRule(inLocationRule)
			,mRotationRule(inRotationRule)
			,mScaleRule(inScaleRule)
			,bCallModify(bInCallModify)
		{

		}

		DetachmentTransformRules(const AttachmentTransformRules& attachmentRules, bool bInCallModify)
			:mLocationRule(attachmentRules.mLocationRule == EAttachmentRule::KeepRelative ? EDetachmentRule::KeepRelative : EDetachmentRule::KeepWorld)
			,mRotationRule(attachmentRules.mRotationRule == EAttachmentRule::KeepRelative ? EDetachmentRule::KeepRelative:EDetachmentRule::KeepWorld)
			,mScaleRule(attachmentRules.mScaleRule == EAttachmentRule::KeepRelative? EDetachmentRule::KeepRelative:EDetachmentRule::KeepWorld)
			,bCallModify(bInCallModify)
		{

		}

		EDetachmentRule mLocationRule;
		EDetachmentRule mRotationRule;
		EDetachmentRule mScaleRule;
		bool bCallModify;
	};

	namespace EEndPlayReason
	{
		enum Type
		{
			Destroyed,
			LevelTransition,
			EndPlayInEditor,
			RemovedFromWorld,
			Quit,
		};
	}

	struct RepAttachment
	{
		class AActor* mAttachParent;
		Rotator mRotationOffset;
		wstring mAttachSocket;

		class SceneComponent* mAttachComponent;

		RepAttachment()
			:mAttachParent(nullptr)
			,mRotationOffset(ForceInit)
			,mAttachSocket(Name_None)
			,mAttachComponent(nullptr)
		{}

	};

	namespace EComponentMobility
	{
		enum Type
		{
			Static,
			Stationary,
			Movable
		};
	}

	enum class ELevelCollectionType
	{
		DynamicSourceLevels,
		DynamicDunplicatedLevels,
		StaticLevels,
	};

	inline uint8 getDefaultLightingChannelMask()
	{
		return 1;
	}

	struct MeshBuildSettings
	{
		bool bUseMikkTSpace{ false };

		bool bRecomputeNormals{ false };

		bool bRecomputeTangents{ false };

		bool bRemoveDegenerates{ true };

		bool bBuildAdjacencyBuffer{ true };

		bool bBuildReversedIndexBuffer{ true };

		bool bUseHighPrecisionTangentBasis{ false };

		bool bUseFullPrecisionUVs{ false };

		float3 mBuildScale3D{ float3::One };
	};

	enum EMaterialSamplerType
	{
		SamplerType_Color,
		SamplerType_Grayscale,
		SamplerType_Alpha,
		SamplerType_Normal,
		SamplerType_Masks,
		SamplerType_DistanceFieldFont,
		SamplerType_LinearColor,
		SamplerType_LinearGrayscale,
		SamplerType_Max,
	};
}