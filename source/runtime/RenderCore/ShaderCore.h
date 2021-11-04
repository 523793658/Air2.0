#pragma once
#include "CoreMinimal.h"
namespace Air
{
	struct ShaderCompilerResourceTable
	{
		uint32 mResourceTableBits{ 0 };
		uint32 mMaxBoundResourceTable{ 0 };
		TArray<uint32> mTextureMap;
		TArray<uint32> mShaderResourceViewMap;
		TArray<uint32> mSamplerMap;
		TArray<uint32> mUnorderedAccessViewMap;
		TArray<uint32> mResourceTableLayoutHashes;
	};
	struct CachedConstantBufferDeclaration;


	extern void generateReferencedConstantBuffers(const TCHAR* sourceFile, const TCHAR* shaderTypeName, const TMap<wstring, TArray<const TCHAR*>>& shaderFileToConstantBufferVaribles, TMap<const TCHAR*, CachedConstantBufferDeclaration>& constantBufferEntries);
}