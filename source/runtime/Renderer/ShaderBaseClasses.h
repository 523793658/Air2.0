#pragma once
#include "RendererMininal.h"
#include "Materials/MeshMaterialShader.h"
#include "RHIResource.h"
#include "Shader.h"
#include "DrawingPolicy.h"
namespace Air
{
	class BaseHS : public MeshMaterialShader
	{
		DECLARE_SHADER_TYPE(BaseHS, MeshMaterial);
	public:
		static bool shouldCache(EShaderPlatform platform, const FMaterial* material, const VertexFactoryType * vertexFactoryType)
		{
			if (!RHISupportsTessellation(platform))
			{
				return false;
			}
			if (vertexFactoryType && !vertexFactoryType->supportsTessellationShaders())
			{
				return false;
			}
			if (!material || material->getTessellationMode() == MTM_NOTessellation)
			{
				return false;
			}
			return true;
		}

		BaseHS(const ShaderMetaType::CompiledShaderInitializerType & initializer)
			:MeshMaterialShader(initializer)
		{

		}

		BaseHS() {}

		void setParameters(RHICommandList& RHICmdList, const MaterialRenderProxy* materialRenderProxy, const SceneView& view)
		{
			MeshMaterialShader::setParameters(RHICmdList, (HullShaderRHIParamRef)getHullShader(), materialRenderProxy, *materialRenderProxy->getMaterial(view.getFeatureLevel()), view, view.mViewConstantBuffer, ESceneRenderTargetsMode::SetTextures);
		}

		void setMesh(RHICommandList& RHICmdList, const VertexFactory* vertexFactory, const SceneView& view, const PrimitiveSceneProxy* proxy, const MeshBatchElement& batchElement, const DrawingPolicyRenderState& drawRenderState)
		{
			MeshMaterialShader::setMesh(RHICmdList, (HullShaderRHIParamRef)getHullShader(), vertexFactory, view, proxy, batchElement, drawRenderState);
		}
	};

	class BaseDS : public MeshMaterialShader
	{
		DECLARE_SHADER_TYPE(BaseDS, MeshMaterial);
	public:
		static bool shouldCache(EShaderPlatform platform, const FMaterial* material, const VertexFactoryType* vertexFactoryType)
		{
			if (!RHISupportsTessellation(platform))
			{
				return false;
			}
			if (vertexFactoryType && !vertexFactoryType->supportsTessellationShaders())
			{
				return false;
			}
			if (!material || material->getTessellationMode() == MTM_NOTessellation)
			{
				return false;
			}
			return true;
		}
		BaseDS(const ShaderMetaType::CompiledShaderInitializerType & initializer)
			:MeshMaterialShader(initializer)
		{}
		BaseDS() {}
		void setParameters(RHICommandList& RHICmdList, const MaterialRenderProxy* materialRenderProxy, const SceneView& view)
		{
			MeshMaterialShader::setParameters(RHICmdList, (DomainShaderRHIParamRef)getDomainShader(), materialRenderProxy, *materialRenderProxy->getMaterial(view.getFeatureLevel()), view, view.mViewConstantBuffer, ESceneRenderTargetsMode::SetTextures);
		}

		void setMesh(RHICommandList& RHICmdList, const VertexFactory* vertexFactory, const SceneView& view, const PrimitiveSceneProxy* proxy, const MeshBatchElement& batchElement, const DrawingPolicyRenderState& drawRenderState)
		{
			MeshMaterialShader::setMesh(RHICmdList, (DomainShaderRHIParamRef)getDomainShader(), vertexFactory, view, proxy, batchElement, drawRenderState);
		}
	};
}