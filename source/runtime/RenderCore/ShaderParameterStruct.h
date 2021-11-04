#pragma once
namespace Air
{

	template<typename ParameterStruct>
	void bindForLegacyShaderParameters(Shader* shader, int32 permutationId, const ShaderParameterMap& parameterMap, bool bShouldBindEverything = false)
	{
		shader->mBindings.bindForLegacyShaderParameters(shader, permutationId, parameterMap, *ParameterStruct::TypeInfo::getStructMetadata(), bShouldBindEverything);
	}



#define SHADER_USE_PARAMETER_STRUCT_INTERNAL(ShaderClass, ShaderParentClass, bShouldBindEverything) \
	ShaderClass(const ShaderMetaType::CompiledShaderInitializerType& initializer) \
		: ShaderParentClass(initializer) \
	{\
		bindForLegacyShaderParameters<Parameters>(this, initializer.mPermutationId, initializer.mParameterMap, bShouldBindEverything); \
	}\
	\
	ShaderClass() \
	{}\


#define SHADER_USE_PARAMETER_STRUCT(ShaderClass, ShaderParentClass) \
	SHADER_USE_PARAMETER_STRUCT_INTERNAL(ShaderClass, ShaderParentClass, true) \
	\
	static inline const ShaderParametersMetadata* getRootParametersMetadata() {return Parameters::TypeInfo::getStructMetadata();}



}