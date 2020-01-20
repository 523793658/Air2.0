#include "HdrCustomResolveShaders.h"
namespace Air
{
	IMPLEMENT_SHADER_TYPE(, HdrCustomResolveVS, TEXT("HdrCustomResolveShaders"), TEXT("HdrCustomResolveVS"), SF_Vertex);

	IMPLEMENT_SHADER_TYPE(, HdrCustomResolve2xPS, TEXT("HdrCustomResolveShaders"), TEXT("HdrCustomResolve2xPS"), SF_Pixel);

	IMPLEMENT_SHADER_TYPE(, HdrCustomResolve4xPS, TEXT("HdrCustomResolveShaders"), TEXT("HdrCustomResolve4xPS"), SF_Pixel);

	IMPLEMENT_SHADER_TYPE(, HdrCustomResolve8xPS, TEXT("HdrCustomResolveShaders"), TEXT("HdrCustomResolve8xPS"), SF_Pixel);

	IMPLEMENT_SHADER_TYPE(, HdrCustomResolveMask2xPS, TEXT("HdrCustomResolveShaders"), TEXT("HdrCustomResolveMaskPS"), SF_Pixel);
	IMPLEMENT_SHADER_TYPE(, HdrCustomResolveMask4xPS, TEXT("HdrCustomResolveShaders"), TEXT("HdrCustomResolveMaskPS"), SF_Pixel);
	IMPLEMENT_SHADER_TYPE(, HdrCustomResolveMask8xPS, TEXT("HdrCustomResolveShaders"), TEXT("HdrCustomResolveMaskPS"), SF_Pixel);

}