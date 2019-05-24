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

	void ColorVertexBuffer::init(const TArray<StaticMeshBuildVertex>& vertices)
	{
		const int32 inVertexCount = vertices.size();
		bool bAllColorsAreOpaqueWhite = true;
		bool bAllColorsAreEquals = true;

		if (inVertexCount > 0)
		{
			const Color firstColor = vertices[0].mColor;
			for (int32 curVertexIndex = 0; curVertexIndex < inVertexCount; ++curVertexIndex)
			{
				const Color curColor = vertices[curVertexIndex].mColor;
				if (curColor.R != 255 || curColor.G != 255 || curColor.B != 255 || curColor.A != 255)
				{
					bAllColorsAreOpaqueWhite = false;
				}

				if (curColor.R != firstColor.R || curColor.G != firstColor.G || curColor.B != firstColor.B || curColor.A != firstColor.A)
				{
					bAllColorsAreEquals = false;
				}

				if (!bAllColorsAreEquals && !bAllColorsAreOpaqueWhite)
				{
					break;
				}
			}
		}

		if (bAllColorsAreOpaqueWhite)
		{
			cleanUp();
			mStride = 0;
			mNumVertex = 0;
		}
		else
		{
			mNumVertex = inVertexCount;
			allocateData();

			mVertexData->resizeBuffer(mNumVertex);
			mData = mVertexData->getDataPointer();

			for (int32 vertexIndex = 0; vertexIndex < vertices.size(); vertexIndex++)
			{
				const StaticMeshBuildVertex& sourceVertex = vertices[vertexIndex];
				const uint32 destVertexIndex = vertexIndex;
				vertexColor(destVertexIndex) = sourceVertex.mColor;
			}
		}
	}

	void ColorVertexBuffer::initRHI()
	{
		if (mVertexData != nullptr)
		{
			ResourceArrayInterface* resourceArray = mVertexData->getResourceArray();
			if (resourceArray->getResourceDataSize())
			{
				RHIResourceCreateInfo info(resourceArray);
				mVertexBufferRHI = RHICreateVertexBuffer(resourceArray->getResourceDataSize(), BUF_Static, info);
			}
		}
	}
}