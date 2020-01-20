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
		void GameThread_UpdateContents(const Guid& inId, const TArray<float4>& data, const wstring& inOwnerName, bool bRecreateConstantBuffer);

		void GameThread_Destroy();

		Guid getID() const
		{
			return mID;
		}
		RHIConstantBuffer* getConstantBuffer() const
		{
			return mConstantBuffer;
		}

		MaterialParameterCollectionInstanceResource();
		~MaterialParameterCollectionInstanceResource();
	private:
		Guid mID;
		wstring mOwnerName;
		ConstantBufferRHIRef mConstantBuffer;
		RHIConstantBufferLayout mConstantBufferLayout;
		void updateContents(const Guid& inId, const TArray<float4>& indata, const wstring& inOwnerName, bool bRecreateConstantBuffer);
	};


	extern ENGINE_API TMap<Guid, MaterialParameterCollectionInstanceResource*> GDefaultMaterialParameterCollectionInstances;
}