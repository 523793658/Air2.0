#include "LocalVertexFactory.h"
#include "MeshBatch.h"
#include "Rendering/ColorVertexBuffer.h"
#include "Classes/GameFramework/Actor.h"
#include "RenderUtils.h"
namespace Air
{
	IMPLEMENT_VERTEX_FACTORY_TYPE_EX(LocalVertexFactory, "LocalVertexFactory", true, true, true, true, true, true, true);

	void LocalVertexFactory::initRHI()
	{
		BOOST_ASSERT(hasValidFeatureLevel());
		const bool bCanUseGPUScene = useGPUScene(GMaxRHIShaderPlatform, GMaxRHIFeatureLevel);

		if (mData.mPositionComponents.mVertexBuffer != mData.mTangentBasisComponents[0].mVertexBuffer)
		{
			auto addDeclaration = [this, bCanUseGPUScene](EVertexInputStreamType inputStreamType, bool bAddNormal)
			{
				VertexDeclarationElementList streamElements;
				streamElements.add(accessStreamComponent(mData.mPositionComponent, 0, inputStreamType));

				bAddNormal = bAddNormal && mData.mTangentBasisComponents[1].mVertexBuffer != nullptr;
				if (bAddNormal)
				{
					streamElements.add(accessStreamComponent(mData.mTangentBasisComponents[1], 2, inputStreamType));
				}

				const uint8 typeIndex = static_cast<uint8>(inputStreamType);
				mPrimitiveIdStreamIndex[typeIndex] = -1;
			};
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

		if (mData.mTextureCoordinates.size())
		{
			const int32 baseTexCoordAttribute = 4;
			for (int32 coordinateIndex = 0; coordinateIndex < mData.mTextureCoordinates.size(); coordinateIndex++)
			{
				elements.add(accessStreamComponent(mData.mTextureCoordinates[coordinateIndex], baseTexCoordAttribute + coordinateIndex));
			}

			for (int32 coordinateIndex = mData.mTextureCoordinates.size(); coordinateIndex < MAX_STATIC_TEXCOORDS / 2; coordinateIndex++)
			{
				elements.add(accessStreamComponent(mData.mTextureCoordinates[mData.mTextureCoordinates.size() - 1], baseTexCoordAttribute + coordinateIndex));
			}
		}
		if (mData.mLightMapCoordinateComponent.mVertexBuffer)
		{
			elements.add(accessStreamComponent(mData.mLightMapCoordinateComponent, 15));
		}
		else if (mData.mTextureCoordinates.size())
		{
			elements.add(accessStreamComponent(mData.mTextureCoordinates[0], 15));
		}
		BOOST_ASSERT(mStreams.size() > 0);
		initDeclaration(elements);
		BOOST_ASSERT(isValidRef(getDeclaration()));
	}

	bool LocalVertexFactory::shouldCompilePermutation(EShaderPlatform platform, const class FMaterial* material, const class ShaderType* shaderType)
	{
		return true;
	}

	void LocalVertexFactory::setData(const DataType& inData)
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

	void LocalVertexFactory::validateCompiledResult(const VertexFactoryType* type, EShaderPlatform platform, const ShaderParameterMap& parameterMap, TArray<wstring>& outErrors)
	{
		if (type->supportsPrimitiveIdStream() && useGPUScene(platform, getMaxSupportedFeatureLevel(platform)) && parameterMap.containsParameterAllocation(PrimitiveConstantShaderParameters::StaticStructMetadata.getShaderVariableName()))
		{
			outErrors.addUnique(String::printf(TEXT("Shader attempted to bind the primitive constant buffer event though vertex factory %s computes a primitiveId per-instance. This will break auto-instancing. Shaders should use GetPrimtiveData(Parameters.PrimitiveId).Member instead of Primitive.Member."), type->getName()));
		}
	}
}