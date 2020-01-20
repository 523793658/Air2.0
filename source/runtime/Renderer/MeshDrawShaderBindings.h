#pragma once
#include "CoreMinimal.h"
#include "RHIDefinitions.h"
#include "Shader.h"
#include "Misc/Crc.h"
namespace Air
{
	class MeshDrawShaderBindingsLayout
	{
	public:
		EShaderFrequency mFrequency : SF_NumBits + 1;
		const ShaderParameterMapInfo& mParameterMapInfo;

		MeshDrawShaderBindingsLayout(const Shader* shader)
			:mParameterMapInfo(shader->getParameterMapInfo())
		{
			BOOST_ASSERT(shader);
			mFrequency = (EShaderFrequency)shader->getTarget().mFrequency;
			BOOST_ASSERT((EShaderFrequency)mFrequency == (EShaderFrequency)shader->getTarget().mFrequency);
		}

		inline uint32 getHash() const
		{
			uint32 localFrequency = mFrequency;
			return Crc::typeCrc32(localFrequency, 0);
		}

		bool operator == (const MeshDrawShaderBindingsLayout& rhs) const
		{
			return mFrequency == rhs.mFrequency && mParameterMapInfo == rhs.mParameterMapInfo;
		}

		inline uint32 getDataSizeBytes() const
		{
			uint32 dataSize = sizeof(void*) * (mParameterMapInfo.mConstantBuffers.size() + mParameterMapInfo.mTextureSamplers.size() + mParameterMapInfo.mSRVs.size());

			dataSize += Math::divideAndRoundUp(mParameterMapInfo.mSRVs.size(), 8);

			for (int32 looseBufferIndex = 0; looseBufferIndex < mParameterMapInfo.mLooseParameterBuffers.size(); looseBufferIndex++)
			{
				dataSize += mParameterMapInfo.mLooseParameterBuffers[looseBufferIndex].mBufferSize;
			}

			return align(dataSize, sizeof(void*));
		}
	protected:
		inline uint32 getConstantBufferOffset() const
		{
			return 0;
		}

		inline uint32 getSamplerOffset() const
		{
			return mParameterMapInfo.mConstantBuffers.size() * sizeof(RHIConstantBuffer*);
		}

		inline uint32 getSRVOffset() const
		{
			return getSamplerOffset() + mParameterMapInfo.mTextureSamplers.size() * sizeof(RHISamplerState*);
		}

		inline uint32 getSRVTypeOffset() const
		{
			return getSRVOffset() + mParameterMapInfo.mSRVs.size() * sizeof(RHIShaderResourceView*);
		}

		inline uint32 getLooseDataOffset() const
		{
			return getSRVTypeOffset() + Math::divideAndRoundUp(mParameterMapInfo.mSRVs.size(), 8);
		}
		friend class MeshDrawShaderBindings;
	};


	class MeshDrawSingleShaderBindings : public MeshDrawShaderBindingsLayout
	{
		friend class MeshDrawShaderBindings;
	};
}