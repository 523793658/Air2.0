
#define USES_GBUFFER		(FEATURE_LEVEL >= FEATURE_LEVEL_SM4 && (MATERIALBLENDING_SOLID || MATERIALBLENDING_MASKED) && !SIMPLE_FORWARD_SHADING && !FORWARD_SHADING)




struct SharedBasePassInterpolants
{
	

};


struct BasePassInterpolantsVSToPS : SharedBasePassInterpolants
{
};