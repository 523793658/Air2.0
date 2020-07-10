#pragma  once
#include "CoreMinimal.h"
#include "ShaderCore.h"

namespace Air
{
	enum EShaderStage
	{
		SHADER_STAGE_VERTEX = 0,
		SHADER_STAGE_PIXEL,
		SHADER_STAGE_GEOMETRY,
		SHADER_STAGE_NULL,
		SHADER_STAGE_DOMAIN,
		NUM_NON_COMPUTE_SHADER_STAGE,
		SHADER_STAGE_COMPUTE = NUM_NON_COMPUTE_SHADER_STAGE,
		NUM_SHADER_STAGES
	};

	enum class EPackedTypeName : int8
	{
		HighP	= 'h',
		MediumP = 'm',
		LowP	= 'l',
		Int		= 'i',
		Uint	= 'u',
		Invalide = ' ',
	};

	enum class EPackedTypeIndex : int8
	{
		HighP = 0,
		MediumP = 1,
		LowP = 2,
		Int	 = 3,
		Uint = 4,

		Max = 5,
		Invalide = -1
	};

	enum
	{
		PACKED_TYPENAME_HIGHP		=(int32)EPackedTypeName::HighP,
		PACKED_TYPENAME_MEDIUMP		=(int32)EPackedTypeName::MediumP,
		PACKED_TYPENAME_LOWP		=(int32)EPackedTypeName::LowP,
		PACKED_TYPENAME_INT		=(int32)EPackedTypeName::Int,
		PACKED_TYPENAME_UINT		=(int32)EPackedTypeName::Uint,
		PACKED_TYPENAME_SAMPLER		='s',
		PACKED_TYPENAME_IMAGE		='g',

		PACKED_TYPEINDEX_HIGHP		= (int32)EPackedTypeIndex::HighP,
		PACKED_TYPEINDEX_MEDIUMP		= (int32)EPackedTypeIndex::MediumP,
		PACKED_TYPEINDEX_LOWP		= (int32)EPackedTypeIndex::LowP,
		PACKED_TYPEINDEX_INT		= (int32)EPackedTypeIndex::Int,
		PACKED_TYPEINDEX_UINT		= (int32)EPackedTypeIndex::Uint,
		PACKED_TYPEINDEX_MAX		= (int32)EPackedTypeIndex::Max,

	};

	static FORCEINLINE uint8 shaderStageIndexToTypeName(uint8 shaderStage)
	{
		switch (shaderStage)
		{
		case SHADER_STAGE_VERTEX: return 'v';
		case SHADER_STAGE_PIXEL:	return 'p';
		case SHADER_STAGE_GEOMETRY:	return 'g';
		case SHADER_STAGE_NULL:		return 'h';
		case SHADER_STAGE_DOMAIN:	return 'd';
		case SHADER_STAGE_COMPUTE:	return 'c';
		default:
			break;
		}
		BOOST_ASSERT(false);
		return 0;
	}

	static FORCEINLINE uint8 packedTypeIndexToTypeName(uint8 arrayType)
	{
		switch (arrayType)
		{
		case PACKED_TYPEINDEX_HIGHP: return PACKED_TYPENAME_HIGHP;
		case PACKED_TYPEINDEX_MEDIUMP: return PACKED_TYPENAME_MEDIUMP;
		case PACKED_TYPEINDEX_LOWP: return PACKED_TYPENAME_LOWP;
		case PACKED_TYPEINDEX_INT: return PACKED_TYPENAME_INT;
		case PACKED_TYPEINDEX_UINT: return PACKED_TYPENAME_UINT;
		default:
			break;
		}
		BOOST_ASSERT(false);
		return 0;
	}

	static FORCEINLINE uint8 packedTypeNameToTypeIndex(uint8 arrayName)
	{
		switch (arrayName)
		{
		case PACKED_TYPENAME_HIGHP: return  PACKED_TYPEINDEX_HIGHP;
		case PACKED_TYPENAME_MEDIUMP: return  PACKED_TYPEINDEX_MEDIUMP;
		case PACKED_TYPENAME_LOWP: return PACKED_TYPEINDEX_LOWP;
		case PACKED_TYPENAME_INT: return PACKED_TYPEINDEX_INT;
		case PACKED_TYPENAME_UINT: return PACKED_TYPEINDEX_UINT;
		default:
			break;
		}
		BOOST_ASSERT(false);
		return 0;
	}

	static FORCEINLINE bool isValidPackedTypeName(EPackedTypeName typeName)
	{
		switch (typeName)
		{
		case Air::EPackedTypeName::HighP:
		case Air::EPackedTypeName::MediumP:
		case Air::EPackedTypeName::LowP:
		case Air::EPackedTypeName::Int:
		case Air::EPackedTypeName::Uint:
			return true;
		default:
			break;
		}
		return false;
	}

	static FORCEINLINE EPackedTypeName packedTypeIndexToTypeName(EPackedTypeIndex typeIndex)
	{
		switch (typeIndex)
		{
		case Air::EPackedTypeIndex::HighP:
			return EPackedTypeName::HighP;
		case Air::EPackedTypeIndex::MediumP:
			return EPackedTypeName::MediumP;
		case Air::EPackedTypeIndex::LowP:
			return EPackedTypeName::LowP;
		case Air::EPackedTypeIndex::Int:
			return EPackedTypeName::Int;
		case Air::EPackedTypeIndex::Uint:
			return EPackedTypeName::Uint;
		default:
			break;
		}
		return EPackedTypeName::Invalide;
	}

	static FORCEINLINE EPackedTypeIndex packedTypeNameToTypeIndex(EPackedTypeName typeName)
	{
		switch (typeName)
		{
		case Air::EPackedTypeName::HighP:
			return EPackedTypeIndex::HighP;
		case Air::EPackedTypeName::MediumP:
			return EPackedTypeIndex::MediumP;
		case Air::EPackedTypeName::LowP:
			return EPackedTypeIndex::LowP;
		case Air::EPackedTypeName::Int:
			return EPackedTypeIndex::Int;
		case Air::EPackedTypeName::Uint:
			return EPackedTypeIndex::Uint;
		default:
			break;
		}
		return EPackedTypeIndex::Invalide;
	}

	struct PackedArrayInfo
	{
		uint16 mSize;
		uint8 mTypeName;
		uint8 mTypeIndex;
	};

	inline Archive& operator<<(Archive& ar, PackedArrayInfo& info)
	{
		ar << info.mSize;
		ar << info.mTypeName;
		ar << info.mTypeIndex;
		return ar;
	}


	struct ShaderBindings
	{
		TArray<TArray<PackedArrayInfo>> mPackedConstantBuffers;
		TArray<PackedArrayInfo> mPackedGlobalArrays;
		ShaderCompilerResourceTable mShaderResourceTable;

		uint16 mInOutMask;
		uint8 mNumSamplers;
		uint8 mNumConstantBuffers;
		uint8 mNumUAVs;
		bool bHasRegularCosntantBuffers;
	};

	struct ConstantBufferCopyInfo
	{
		uint16 mSourceOffsetInFloats;
		uint8 mSourceCBIndex;
		uint8 mDestCBIndex;
		uint8 mDestCBTypeName;
		uint8 mDestCBTypeIndex;
		uint16 mDestOffsetInFloats;
		uint16 mSizeInFloats;
	};

	inline Archive& operator<<(Archive& ar, ConstantBufferCopyInfo& info)
	{
		ar << info.mSourceOffsetInFloats;
		ar << info.mSourceCBIndex;
		ar << info.mDestCBIndex;
		ar << info.mDestCBTypeName;
		if (ar.isLoading())
		{
			info.mDestCBTypeIndex = packedTypeNameToTypeIndex(info.mDestCBTypeName);
		}
		ar << info.mDestOffsetInFloats;
		ar << info.mSizeInFloats;
		return ar;
	}
}