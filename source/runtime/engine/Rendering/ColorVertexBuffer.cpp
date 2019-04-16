#include "ColorVertexBuffer.h"
#include "StaticMeshVertexData.h"
namespace Air
{
	class ColorVertexData : public TStaticMeshVertexData<Color>
	{
	public:
		ColorVertexData(bool inNeedsCPUAccess = false)
			:TStaticMeshVertexData<Color>(inNeedsCPUAccess)
		{}

		TStaticMeshVertexData<Color>& operator = (const TArray<Color>& other)
		{
			TStaticMeshVertexData<Color>::operator=(other);
			return *this;
		}
	};

	ColorVertexBuffer::ColorVertexBuffer()
	{

	}

	ColorVertexBuffer::~ColorVertexBuffer()
	{
		cleanUp();
	}

	void ColorVertexBuffer::cleanUp()
	{
		if (mVertexData)
		{
			delete mVertexData;
			mVertexData = nullptr;
		}
	}

	void ColorVertexBuffer::serialize(Archive& ar, bool bNeedsCPUAccess)
	{
		if (ar.isSaving() && mNumVertex > 0 && mVertexData == nullptr)
		{
			int32 serializedStride = 0;
			int32 serialziedNumVertices = 0;
			ar << serializedStride << serialziedNumVertices;
		}
		else
		{
			ar << mStride << mNumVertex;
			if (ar.isLoading() && mNumVertex > 0)
			{
				allocateData(bNeedsCPUAccess);
			}
			if (mVertexData != nullptr)
			{
				mVertexData->serialize(ar);
				if (mVertexData->size() > 0)
				{
					mData = mVertexData->getDataPointer();
				}
			}
		}
	}

	void ColorVertexBuffer::allocateData(bool bNeedsCPUAccess /* = true */)
	{
		cleanUp();
		mVertexData = new ColorVertexData(bNeedsCPUAccess);
		mStride = mVertexData->getStride();
	}
}