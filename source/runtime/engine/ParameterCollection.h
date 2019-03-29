#pragma once
#include "EngineMininal.h"
#include "Containers/Map.h"
#include "RHIResource.h"
#include "Misc/Guid.h"
namespace Air
{
	static const uint32 MaxNumParameterCollectionsPerMaterial = 2;
	class MaterialParameterCollectionInstanceResource
	{
	public:
		void GameThread_UpdateContents(const Guid& inId, const TArray<float4>& data);

		void GameThread_Destroy();

		Guid getID() const
		{
			return mID;
		}
		ConstantBufferRHIParamRef getConstantBuffer() const
		{
			return mConstantBuffer;
		}

		MaterialParameterCollectionInstanceResource();
		~MaterialParameterCollectionInstanceResource();
	private:
		Guid mID;
		ConstantBufferRHIRef mConstantBuffer;
		RHIConstantBufferLayout mConstantBufferLayout;
		void updateContents(const Guid& inId, const TArray<float4>& indata);
	};


	extern ENGINE_API TMap<Guid, MaterialParameterCollectionInstanceResource*> GDefaultMaterialParameterCollectionInstances;
}