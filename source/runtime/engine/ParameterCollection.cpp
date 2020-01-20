#include "ParameterCollection.h"
#include "SimpleReflection.h"
#include "Classes/Materials/MaterialParameterCollection.h"
#include "RenderingThread.h"
#include "Classes/Engine/World.h"
#include "ShaderParameters.h"
namespace Air
{


	TMap<Guid, MaterialParameterCollectionInstanceResource*> GDefaultMaterialParameterCollectionInstances;
	
	MaterialParameterCollection::MaterialParameterCollection(const ObjectInitializer& objectInitializer/* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		mDefaultResource = nullptr;
	}

	ShaderConstantBufferParameter* constructCollectionConstantBufferParameter() { return nullptr; }

	void MaterialParameterCollection::createBufferStruct()
	{
		TArray<ShaderParametersMetadata::Member> members;
		uint32 nextMemberOffset = 0;
		const uint32 numVectors = Math::divideAndRoundUp(mScalarParameters.size(), 4) + mVectorParameters.size();
		new(members)ShaderParametersMetadata::Member(TEXT("Vectors"), TEXT(""), nextMemberOffset, CBMT_FLOAT32, EShaderPrecisionModifier::Half, 1, 4, numVectors, nullptr);
		const uint32 vectorArraySize = numVectors * sizeof(float4);
		nextMemberOffset += vectorArraySize;
		static wstring layoutName(TEXT("MaterialCollection"));
		const uint32 structSize = align(nextMemberOffset, CONSTANT_BUFFER_STRUCT_ALIGNMENT);
		mShaderParametersMetadata = makeUniquePtr<ShaderParametersMetadata>(
			ShaderParametersMetadata::EUseCase::DataDrivenShaderParameterStruct,
			layoutName,
			TEXT("MaterialCollection"),
			TEXT("MaterialCollection"),
			structSize,
			members);
	}

	void MaterialParameterCollection::getDefaultParametrData(TArray<float4>& parameterData) const
	{
		parameterData.empty(Math::divideAndRoundUp(mScalarParameters.size(), 4) + mVectorParameters.size());
		for (int32 parameterIndex = 0; parameterIndex < mScalarParameters.size(); parameterIndex++)
		{
			const CollectionScalarParameter& parameter = mScalarParameters[parameterIndex];
			if (parameterIndex % 4 == 0)
			{
				parameterData.add(float4(0, 0, 0, 0));
			}
			float4& currentVector = parameterData.last();
			currentVector[parameterIndex % 4] = parameter.mDefaultValue;
		}
		for (int32 parameterIndex = 0; parameterIndex < mVectorParameters.size(); parameterIndex++)
		{
			const CollectionVectorParameter& parameter = mVectorParameters[parameterIndex];
			parameterData.add(parameter.mDefaultValue);
		}
	}

	void MaterialParameterCollection::updateDefaultResource(bool bRecreateConstantBuffer)
	{
		TArray<float4> parameterData;
		getDefaultParametrData(parameterData);
		mDefaultResource->GameThread_UpdateContents(mStateId, parameterData, getName(), bRecreateConstantBuffer);
		Guid id = mStateId;
		MaterialParameterCollectionInstanceResource* resource = mDefaultResource;

		ENQUEUE_RENDER_COMMAND(updateDefaultResourceCommand)(
				[id, resource](RHICommandListImmediate& RHICmdList){
				GDefaultMaterialParameterCollectionInstances.emplace(id, resource);
			});
	}

	void MaterialParameterCollection::postLoad()
	{
		ParentType::postLoad();
		if (!mStateId.isValid())
		{
			mStateId = Guid::newGuid();
		}
		createBufferStruct();

		auto& worlds = World::getAllWorlds();
		for (auto& world : worlds)
		{
			world->addParameterCollectionInstances(this, true);
		}
		updateDefaultResource(true);
	}

	static wstring MaterialParameterCollectionInstanceResourceName(TEXT("MaterialParameterCollectionInstanceResource"));
	MaterialParameterCollectionInstanceResource::MaterialParameterCollectionInstanceResource()
		:mConstantBufferLayout(MaterialParameterCollectionInstanceResourceName)
	{

	}

	MaterialParameterCollectionInstanceResource::~MaterialParameterCollectionInstanceResource()
	{
		BOOST_ASSERT(isInRenderingThread());
		mConstantBuffer.safeRelease();
	}

	void MaterialParameterCollectionInstanceResource::updateContents(const Guid& inId, const TArray<float4>& indata, const wstring& inOwnerName, bool bRecreateConstantBuffer)
	{
		mID = inId;
		mOwnerName = inOwnerName;

		if (inId != Guid() && indata.size() > 0)
		{
			const uint32 newSize = indata.getTypeSize() * indata.size();
			BOOST_ASSERT(mConstantBufferLayout.mResources.size() == 0);

			if (!bRecreateConstantBuffer && isValidRef(mConstantBuffer))
			{
				BOOST_ASSERT(newSize == mConstantBufferLayout.mConstantBufferSize);
				BOOST_ASSERT(mConstantBuffer->getLayout() == mConstantBufferLayout);
				RHIUpdateConstantBuffer(mConstantBuffer, indata.getData());
			}
			else
			{
				mConstantBufferLayout.mConstantBufferSize = newSize;
				mConstantBufferLayout.computeHash();
				mConstantBuffer = RHICreateConstantBuffer(indata.getData(), mConstantBufferLayout, ConstantBuffer_MultiFrame);
			}
		}
	}

	void MaterialParameterCollectionInstanceResource::GameThread_Destroy()
	{
		ENQUEUE_RENDER_COMMAND(
			DestroyCollectionCommand)([this](RHICommandListImmediate& RHICmdList){
				delete this;
			}
		);
	}

	void MaterialParameterCollectionInstanceResource::GameThread_UpdateContents(const Guid& inId, const TArray<float4>& data, const wstring& inOwnerName, bool bRecreateConstantBuffer)
	{
		ENQUEUE_RENDER_COMMAND(
			UpdateCollectionCommand)([inId, data, inOwnerName, bRecreateConstantBuffer, this](RHICommandListImmediate& RHICmdList)
			{
				this->updateContents(inId, data, inOwnerName, bRecreateConstantBuffer);
			}
		);
	}

	DECLARE_SIMPLER_REFLECTION(MaterialParameterCollection);



}