#pragma once
#include "EngineMininal.h"
#include "VertexFactory.h"
#include "Components.h"
namespace Air
{
	class ENGINE_API LocalVertexFactory : public VertexFactory
	{
		DECALRE_VERTEX_FACTORY_TYPE(LocalVertexFactory)
	public:
		LocalVertexFactory()
		{

		}

		struct DataType
		{
			VertexStreamComponent mPositionComponents;

			VertexStreamComponent mTangentBasisComponents[2];

			TArray<VertexStreamComponent, TFixedAllocator<MAX_STATIC_TEXCOORDS / 2>> mTextureCoordinates;

			VertexStreamComponent mLightMapCoordinateComponent;

			VertexStreamComponent mColorComponent;
		};

		static bool shouldCache(EShaderPlatform platform, const class FMaterial* material, const class ShaderType* shaderType);

		static void modifyCompilationEnvironment(EShaderPlatform platform, const FMaterial* material, ShaderCompilerEnvironment& outEnvironment)
		{
		}

		static VertexFactoryShaderParameters* constructShaderParameters(EShaderFrequency shaderFrequency);

		static bool supportsTessellationShaders() { return true; }

		virtual void initRHI() override;

		FORCEINLINE_DEBUGGABLE void setColorOverrideStream(RHICommandList& RHICmdList, const VertexBuffer* colorVertexBuffer) const
		{
			BOOST_ASSERT(colorVertexBuffer->isInitialized());
			BOOST_ASSERT(isInitialized() && mData.mColorComponent.bSetByVertexFactoryInSetMesh && mColorStreamIndex > 0);
			RHICmdList.setStreamSource(mColorStreamIndex, colorVertexBuffer->mVertexBufferRHI, mData.mColorComponent.mStride, 0);
		}

		void setData(const DataType& inData);

	protected:
		DataType mData;
		int32 mColorStreamIndex;
		const DataType& getData() const { return mData; }
	};

	class LocalVertexFactoryShaderParameters : public VertexFactoryShaderParameters
	{
	public:
		virtual void bind(const ShaderParameterMap& parameterMap) override;

		virtual void serialize(Archive& ar) override;

		virtual void setMesh(RHICommandList& RHICmdList, Shader* shader, const VertexFactory* vertexFactory, const SceneView& view, const MeshBatchElement& batchElement, uint32 dataFlags) const override;

		LocalVertexFactoryShaderParameters()
		{

		}

		bool bAnySpeedTreeParamIsBound{ false };
		ShaderParameter mLODParamter;
	};
}