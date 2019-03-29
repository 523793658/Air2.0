#include "LocalVertexFactory.h"
#include "MeshBatch.h"
#include "Rendering/ColorVertexBuffer.h"
#include "Classes/GameFramework/Actor.h"
namespace Air
{
	IMPLEMENT_VERTEX_FACTORY_TYPE(LocalVertexFactory, "LocalVertexFactory", true, true, true, true, true);

	void LocalVertexFactory::initRHI()
	{
		if (mData.mPositionComponents.mVertexBuffer != mData.mTangentBasisComponents[0].mVertexBuffer)
		{
			VertexDeclarationElementList positionOnlyStreamElements;
			positionOnlyStreamElements.add(accessPositionStreamComponent(mData.mPositionComponents, 0));
			initPositionDeclaration(positionOnlyStreamElements);
		}
		VertexDeclarationElementList elements;
		if (mData.mPositionComponents.mVertexBuffer != nullptr)
		{
			elements.add(accessStreamComponent(mData.mPositionComponents, 0));
		}
		uint8 tangentBasisAttributes[2] = { 1, 2 };
		for (int32 axisIndex = 0; axisIndex < 2; axisIndex++)
		{
			if (mData.mTangentBasisComponents[axisIndex].mVertexBuffer != nullptr)
			{
				elements.add(accessStreamComponent(mData.mTangentBasisComponents[axisIndex], tangentBasisAttributes[axisIndex]));
			}
		}

		if (mData.mColorComponent.mVertexBuffer)
		{
			elements.add(accessStreamComponent(mData.mColorComponent, 3));
		}
		else
		{
			VertexStreamComponent nullColorComponent(&GNullColorVertexBuffer, 0, 0, VET_Color);
			elements.add(accessStreamComponent(nullColorComponent, 3));
		}

		mColorStreamIndex = elements.last().mStreamIndex;

		if (mData.mTextureCoodinates.size())
		{
			const int32 baseTexCoordAttribute = 4;
			for (int32 coordinateIndex = 0; coordinateIndex < mData.mTextureCoodinates.size(); coordinateIndex++)
			{
				elements.add(accessStreamComponent(mData.mTextureCoodinates[coordinateIndex], baseTexCoordAttribute + coordinateIndex));
			}

			for (int32 coordinateIndex = mData.mTextureCoodinates.size(); coordinateIndex < MAX_STATIC_TEXCOORDS / 2; coordinateIndex++)
			{
				elements.add(accessStreamComponent(mData.mTextureCoodinates[mData.mTextureCoodinates.size() - 1], baseTexCoordAttribute + coordinateIndex));
			}
		}
		if (mData.mLightMapCoordinateComponent.mVertexBuffer)
		{
			elements.add(accessStreamComponent(mData.mLightMapCoordinateComponent, 15));
		}
		else if (mData.mTextureCoodinates.size())
		{
			elements.add(accessStreamComponent(mData.mTextureCoodinates[0], 15));
		}
		BOOST_ASSERT(mStreams.size() > 0);
		initDeclaration(elements);
		BOOST_ASSERT(isValidRef(getDeclaration()));
	}

	bool LocalVertexFactory::shouldCache(EShaderPlatform platform, const class FMaterial* material, const class ShaderType* shaderType)
	{
		return true;
	}

	void LocalVertexFactory::SetData(const DataType& inData)
	{
		BOOST_ASSERT(isInRenderingThread());
		BOOST_ASSERT((inData.mColorComponent.mType == VET_None) || (inData.mColorComponent.mType == VET_Color));
		mData = inData;
		updateRHI();
	}

	VertexFactoryShaderParameters* LocalVertexFactory::constructShaderParameters(EShaderFrequency shaderFrequency)
	{
		if (shaderFrequency == SF_Vertex)
		{
			return new LocalVertexFactoryShaderParameters();
		}
		return nullptr;
	}


	void LocalVertexFactoryShaderParameters::bind(const ShaderParameterMap& parameterMap)
	{
	}

	void LocalVertexFactoryShaderParameters::serialize(Archive& ar)
	{

	}

	void LocalVertexFactoryShaderParameters::setMesh(RHICommandList& RHICmdList, Shader* shader, const VertexFactory* vertexFactory, const SceneView& view, const MeshBatchElement& batchElement, uint32 dataFlags) const
	{
		if (batchElement.bUserDataIsColorVertexBuffer)
		{
			ColorVertexBuffer* overrideColorVertexBuffer = (ColorVertexBuffer*)batchElement.mUserData;
			BOOST_ASSERT(overrideColorVertexBuffer);
			static_cast<const LocalVertexFactory*>(vertexFactory)->setColorOverrideStream(RHICmdList, overrideColorVertexBuffer);

		}


	}
}