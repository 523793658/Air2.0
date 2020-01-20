#include "HAL/PlatformProperties.h"
#include "Misc/ScopedSlowTask.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Misc/CoreMisc.h"
#include "Serialization/MemoryReader.h"
#include "GlobalShader.h"
#include "Serialization/MemoryWriter.h"
#include "Interface/ITargetPlatformManagerModule.h"
#include "ShaderCompiler.h"
#include "boost/lexical_cast.hpp"
#include "DerivedDataCacheInterface.h"

namespace Air
{

	const int32 mGlobalShaderMapId = 0;

	TShaderMap<GlobalShaderType>* GGlobalShaderMap[SP_NumPlatforms];

	static wstring getGlobalShaderCacheFilename(EShaderPlatform platform)
	{
		return TEXT("GlobalShaderCache-") + legacyShaderPlatformToShaderFormat(platform) + TEXT(".bin");
	}

	
	
	

	wstring getGlobalShaderMapKeyString(const GlobalShaderMapId& shaderMapId, EShaderPlatform platform)
	{
		wstring format = legacyShaderPlatformToShaderFormat(platform);
		wstring shaderMapKeyString = format + TEXT("_") + boost::lexical_cast<wstring>(getTargetPlatformManagerRef().shaderFormatVersion(format)) + TEXT("_");
		shaderMapAppendKeyString(platform, shaderMapKeyString);
		shaderMapId.appendKeyString(shaderMapKeyString);
		return shaderMapKeyString;
	}


	

	
	

	

	TShaderMap<GlobalShaderType>* getGlobalShaderMap(EShaderPlatform platform)
	{
		BOOST_ASSERT(GGlobalShaderMap[platform]);
		return GGlobalShaderMap[platform];
	}

	
	

	GlobalShaderMapId::GlobalShaderMapId(EShaderPlatform platform)
	{
		TArray<ShaderType*> shaderTypes;
		TArray<const ShaderPipelineType*> shaderPipelineTypes;

		for (TLinkedList<ShaderType*>::TIterator shaderTypeIt(ShaderType::getTypeList()); shaderTypeIt; shaderTypeIt.next())
		{
			GlobalShaderType* globalShaderType = shaderTypeIt->getGlobalShaderType();
			if (!globalShaderType)
			{
				continue;
			}

			bool bList = false;

			for (int32 permutationId = 0; permutationId < globalShaderType->getPermutationCount(); permutationId++)
			{
				if (globalShaderType->shouldCompilePermutation(platform, permutationId))
				{
					bList = true;
					break;
				}
			}
			if (bList)
			{
				shaderTypes.add(globalShaderType);
			}
		}
		for (TLinkedList<ShaderPipelineType*>::TIterator shaderPipelineIt(ShaderPipelineType::getTypeList()); shaderPipelineIt; shaderPipelineIt.next())
		{
			const ShaderPipelineType* pipeline = *shaderPipelineIt;
			if (pipeline->isGlobalTypePipeline())
			{
				int32 numStagesNeeded = 0;
				auto& stageTypes = pipeline->getStages();
				for (const ShaderType* shader : stageTypes)
				{
					const GlobalShaderType* globalShaderType = shader->getGlobalShaderType();
					if (globalShaderType->shouldCompilePermutation(platform, 0))
					{
						++numStagesNeeded;
					}
					else
					{
						break;
					}
				}
				if (numStagesNeeded == stageTypes.size())
				{
					shaderPipelineTypes.push_back(pipeline);
				}
			}
		}
		shaderTypes.sort(CompareShaderTypes());

		for (int32 index = 0; index < shaderTypes.size(); index++)
		{
			ShaderType* shaderType = shaderTypes[index];
			ShaderTypeDependency dependency(shaderType, platform);
			mShaderTypeDependencies.push_back(dependency);
		}

		shaderPipelineTypes.sort(CompareShaderPipelineType());
		for (int32 index = 0; index < shaderPipelineTypes.size(); index++)
		{
			const ShaderPipelineType* pipeline = shaderPipelineTypes[index];
			ShaderPipelineTypeDependency dependency;
			dependency.mShaderPipelineType = pipeline;
			dependency.mStagesSourceHash = pipeline->getSourceHash();
			mShaderPipelineTypeDependencies.push_back(dependency);
		}
	}


	void GlobalShaderMapId::appendKeyString(wstring keyString) const
	{
		TMap<const TCHAR*, CachedConstantBufferDeclaration> referenceConstantBuffers;
		for (int32 shaderIndex = 0; shaderIndex < mShaderTypeDependencies.size(); shaderIndex++)
		{
			const ShaderTypeDependency& shaderTypeDependency = mShaderTypeDependencies[shaderIndex];
			keyString += TEXT("_");
			keyString += shaderTypeDependency.mShaderType->getName();
			keyString += shaderTypeDependency.mSourceHash.toString();

			shaderTypeDependency.mShaderType->getSerializationHistory().appendKeyString(keyString);

			const TMap<const TCHAR*, CachedConstantBufferDeclaration>& referencedShaderParametersMetadatasCache = shaderTypeDependency.mShaderType->getReferencedShaderParametersMetadatasCache();
			for (auto& it : referencedShaderParametersMetadatasCache)
			{
				referenceConstantBuffers.emplace(it.first, it.second);
			}
		}
		for (int32 index = 0; index < mShaderPipelineTypeDependencies.size(); ++index)
		{
			const ShaderPipelineTypeDependency& dependency = mShaderPipelineTypeDependencies[index];
			keyString += TEXT("_");
			keyString += dependency.mShaderPipelineType->getName();
			keyString += dependency.mStagesSourceHash.toString();
			for (auto& shaderType : dependency.mShaderPipelineType->getStages())
			{
				const TMap<const TCHAR*, CachedConstantBufferDeclaration>& referencedShaderParametersMetadatasCache = shaderType->getReferencedShaderParametersMetadatasCache();
				for (auto& it : referencedShaderParametersMetadatasCache)
				{
					referenceConstantBuffers.emplace(it.first, it.second);
				}
			}
		}
		{
			TArray<uint8> tempData;
			SerializationHistory serializationHistory;
			MemoryWriter ar(tempData, true);
			ShaderSaveArchive saveArchive(ar, serializationHistory);
			serializeConstantBufferInfo(saveArchive, referenceConstantBuffers);
			serializationHistory.appendKeyString(keyString);
		}
	}

	
	

	GlobalShader::GlobalShader(const ShaderMetaType::CompiledShaderInitializerType& initializer)
		:Shader(initializer)
	{

	}
}