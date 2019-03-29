#pragma once
#include "EngineMininal.h"
#include "RenderResource.h"
#include "Containers/DynamicRHIResourceArray.h"
namespace Air
{


	namespace EIndexBufferStride
	{
		enum Type
		{
			Force16Bit = 1,
			Force32Bit = 2,
			AutoDetect = 3
		};
	}

	class IndexArrayView
	{
	public:
		IndexArrayView()
			:mUntypedIndexData(nullptr)
			,mNumIndices(0)
			,b32Bit(false)
		{
			
		}

		IndexArrayView(const void* inIndexData, int32 inNumIndices, bool bIn32Bit)
			:mUntypedIndexData(inIndexData)
			,mNumIndices(inNumIndices)
			,b32Bit(bIn32Bit)
		{}

		uint32 operator[](int32 i) { return (uint32)(b32Bit ? ((const uint32*)mUntypedIndexData)[i] : ((const uint16*)mUntypedIndexData)[i]); }

		uint32 operator[](int32 i) const { return (uint32)(b32Bit ? ((const uint32*)mUntypedIndexData)[i] : ((const uint16*)mUntypedIndexData)[i]); }

	private:
		const void* mUntypedIndexData;
		int32 mNumIndices;
		bool b32Bit;
	};

	class RawStaticIndexBuffer : public IndexBuffer
	{
	public:
		RawStaticIndexBuffer(bool inNeedsCPUAccess = false);

		ENGINE_API void setIndices(const TArray<uint32>& inIndices, EIndexBufferStride::Type desiredStride);

		ENGINE_API void getCopy(TArray<uint32>& outIndices) const;

		ENGINE_API IndexArrayView getArrayView() const;

		FORCEINLINE int32 getNumIndices() const
		{
			return b32Bit ? (mIndexStorage.size() / 4) : (mIndexStorage.size() / 2);
		}

		FORCEINLINE uint32 getAllocatedSize() const
		{
			return mIndexStorage.getAllocatedSize();
		}

		void serialize(Archive& ar, bool bNeedsCPUAccess);

		virtual void initRHI() override;

		inline bool is32Bit() const { return b32Bit; }

	private:
		TResourceArray<uint8, INDEXBUFFER_ALIGNMENT> mIndexStorage;
		bool b32Bit;
	};
}