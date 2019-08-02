#include "ScreenRendering.h"
namespace Air
{
	TGlobalResource<ScreenVertexDeclaration> GScreenVertexDeclaration;

	IMPLEMENT_SHADER_TYPE(, ScreenVS, TEXT("ScreenVertexShader"), TEXT("Main"), SF_Vertex);
}