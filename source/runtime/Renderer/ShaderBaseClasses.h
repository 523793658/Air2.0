#pragma once
#include "RendererMininal.h"
#include "Classes/Materials/MeshMaterialShader.h"
#include "RHIResource.h"
#include "Shader.h"
namespace Air
{
	class BaseHS : public MeshMaterialShader
	{
		DECLARE_SHADER_TYPE(BaseHS, MeshMaterial);
	public:
		static bool shouldCompilePermutation(const MeshMaterialShaderPermutationParameters& parameters)
		{
			if (!RHISupportsTessellation(parameters.mPlatform))
			{
				return false;
			}
			if (parameters.mVertexFactoryType && !parameters.mVertexFactoryType->supportsTessellationShaders())
			{
				return false;
			}
			if (!parameters.mMaterial || parameters.mMaterial->getTessellationMode() == MTM_NOTessellation)
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
	};

	class BaseDS : public MeshMaterialShader
	{
		DECLARE_SHADER_TYPE(BaseDS, MeshMaterial);
	public:
		static bool shouldCompilePermutation(const MeshMaterialShaderPermutationParameters& parameters)
		{
			if (!RHISupportsTessellation(parameters.mPlatform))
			{
				return false;
			}
			if (parameters.mVertexFactoryType && !parameters.mVertexFactoryType->supportsTessellationShaders())
			{
				return false;
			}
			if (!parameters.mMaterial || parameters.mMaterial->getTessellationMode() == MTM_NOTessellation)
			{
				return false;
			}
			return true;
		}
		BaseDS(const ShaderMetaType::CompiledShaderInitializerType & initializer)
			:MeshMaterialShader(initializer)
		{}
		BaseDS() {}
	};
}