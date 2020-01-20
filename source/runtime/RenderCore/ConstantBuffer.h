#pragma once
#include "CoreType.h"
#include "RenderCore.h"
#include "RHIDefinitions.h"
#include "RHIResource.h"
#include "Containers/LinkList.h"
#include "Template/AlignmentTemplates.h"
#include "Containers/Map.h"
#include "Template/AirTemplate.h"
#include "Containers/StaticArray.h"
#include "RHICommandList.h"
#include "RenderResource.h"
#include "Misc/CoreMisc.h"
#include "ShaderParameterMacros.h"
#include <array>
namespace Air
{
	template<typename TConstantStruct> class TShaderConstantBufferParameter;


	template<typename BufferStruct>
	TConstantBufferRef<BufferStruct> createConstantBufferImmediate(const BufferStruct& value, EConstantBufferUsage usage, EConstantBufferValidation validation = EConstantBufferValidation::ValidateResources)
	{
		return TConstantBufferRef<BufferStruct>::createConstantBufferImmediate(value, usage, validation);
	}

	template<typename TBufferStruct>
	class TConstantBuffer : public RenderResource
	{
	public:
		TConstantBuffer()
			:mBufferUsage(ConstantBuffer_MultiFrame)
			,mContents(nullptr)
		{}
		~TConstantBuffer()
		{
			if (mContents)
			{
				Memory::free(mContents);
			}
		}
		void setContents(const TBufferStruct& newContents)
		{
			setContentsNoUpdate(newContents);
			updateRHI();
		}

		void setContentsToZero()
		{
			if (!mContents)
			{
				mContents = (uint8*)Memory::malloc(sizeof(TBufferStruct), CONSTANT_BUFFER_STRUCT_ALIGNMENT);
			}
			Memory::memzero(mContents, sizeof(TBufferStruct));
			updateRHI();
		}

		virtual void initDynamicRHI() override
		{
			BOOST_ASSERT(isInRenderingThread());
			mConstantBufferRHI.safeRelease();
			if (mContents)
			{
				mConstantBufferRHI = createConstantBufferImmediate<TBufferStruct>(*((const TBufferStruct*)mContents), mBufferUsage);
			}
		}
		virtual void releaseDynamicRHI() override
		{
			mConstantBufferRHI.safeRelease();
		}

		RHIConstantBuffer* getConstantBufferRHI() const
		{
			BOOST_ASSERT(mConstantBufferRHI.getReference());
			return mConstantBufferRHI;
		}


		EConstantBufferUsage mBufferUsage;

	protected:
		void setContentsNoUpdate(const TBufferStruct& newContents)
		{
			BOOST_ASSERT(isInRenderingThread());
			if (!mContents)
			{
				mContents = (uint8*)Memory::malloc(sizeof(TBufferStruct), CONSTANT_BUFFER_STRUCT_ALIGNMENT);
			}
			Memory::memcpy(mContents, &newContents, sizeof(TBufferStruct));
		}
	private:
		ConstantBufferRHIRef mConstantBufferRHI;
		uint8* mContents;
	};



	struct ResourceTableEntry
	{
		wstring mConstantBufferName;
		uint16 mType;
		uint16 mResourceIndex;
	};

	inline Archive& operator << (Archive& ar, ResourceTableEntry& entry)
	{
		ar << entry.mConstantBufferName;
		ar << entry.mType;
		ar << entry.mResourceIndex;
		return ar;
	}


	




	



}