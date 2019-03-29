#include "OneColorShader.h"
namespace Air
{
	BEGIN_CONSTANT_BUFFER_STRUCT(ClearShaderUB, )
		DECLARE_CONSTANT_BUFFER_STRUCT_MEMBER_ARRAY(float4, DrawColorMRT, [8])
	END_CONSTANT_BUFFER_STRUCT(ClearShaderUB)

		IMPLEMENT_CONSTANT_BUFFER_STRUCT(ClearShaderUB, TEXT("ClearShaderUB"));



#define IMPLEMENT_ONECOLORVS(A, B)	typedef TOneColorVS<A, B> TOneColorVS##A##B;	  \
	IMPLEMENT_SHADER_TYPE2(TOneColorVS##A##B, SF_Vertex);

	IMPLEMENT_ONECOLORVS(false, false);
	IMPLEMENT_ONECOLORVS(false, true);
	IMPLEMENT_ONECOLORVS(true, true);
	IMPLEMENT_ONECOLORVS(true, false);
#undef IMPLEMENT_ONECOLORVS
}