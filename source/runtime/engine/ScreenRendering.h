#pragma once
#include "EngineMininal.h"
#include "RenderResource.h"
#include "RHICommandList.h"
#include "GlobalShader.h"
#include "ShaderParameterUtils.h"
namespace Air
{
	struct ScreenVertex
	{
		float2 mPosition;
		float2 mUV;
	};

	class ScreenVertexDeclaration : public RenderResource
	{
	public:
		VertexDeclarationRHIRef mVertexDeclarationRHI;
		virtual ~ScreenVertexDeclaration() {}

		virtual void initRHI() override
		{
			VertexDeclarationElementList elements;
			uint32 stride = sizeof(ScreenVertex);
			elements.add(VertexElement(0, STRUCT_OFFSET(ScreenVertex, mPosition), VET_Float2, 0, stride));
			elements.add(VertexElement(0, STRUCT_OFFSET(ScreenVertex, mUV), VET_Float2, 1, stride));
			mVertexDeclarationRHI = RHICreateVertexDeclaration(elements);
		}

		virtual void releaseRHI() override
		{
			mVertexDeclarationRHI.safeRelease();
		}
	};

	extern ENGINE_API TGlobalResource<ScreenVertexDeclaration> GScreenVertexDeclaration;

	class ScreenVS : public GlobalShader
	{
		DECLARE_EXPORTED_SHADER_TYPE(ScreenVS, Global, ENGINE_API);
	public:
		static bool shouldCompilePermutation(const GlobalShaderPermutationParameters& parameter) { return true; }

		ScreenVS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{

		}

		ScreenVS() {}

		void setParameters(RHICommandList& RHICmdList, RHIConstantBuffer* viewConstantBuffer)
		{
			GlobalShader::setParameters<ViewConstantShaderParameters>(RHICmdList, getVertexShader(), viewConstantBuffer);
		}

		virtual bool serialize(Archive& ar) override
		{
			return GlobalShader::serialize(ar);
		}
	};

	class ScreenPS : public GlobalShader
	{
		DECLARE_EXPORTED_SHADER_TYPE(ScreenPS, Global, ENGINE_API);
	public:	
		static bool shouldCache(EShaderPlatform platform) { return true; }

		ScreenPS(const ShaderMetaType::CompiledShaderInitializerType& initializer)
			:GlobalShader(initializer)
		{
			inTexture.bind(initializer.mParameterMap, TEXT("inTexture"), SPF_Mandatory);
			inTextureSampler.bind(initializer.mParameterMap, TEXT("inTextureSampler"));
		}

		ScreenPS() {}

		void setParameters(RHICommandList& RHICmdList, const Texture* texture)
		{
			setTextureParameter(RHICmdList, getPixelShader(), inTexture, inTextureSampler, texture);
		}

		void setParameters(RHICommandList& RHICmdList, RHISamplerState* samplerStateRHI, RHITexture* textureRHI)
		{
			setTextureParameter(RHICmdList, getPixelShader(), inTexture, inTextureSampler, samplerStateRHI, textureRHI);
		}

		virtual bool serialize(Archive& ar) override
		{
			bool bShaderHasOutdateParameters = GlobalShader::serialize(ar);
			ar << inTexture;
			ar << inTextureSampler;
			return bShaderHasOutdateParameters;
		}

	private:
		ShaderResourceParameter inTexture;
		ShaderResourceParameter inTextureSampler;
	};
}