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
		TArray<ConstantBufferStruct::Member> members;
		uint32 nextMemberOffset = 0;
		const uint32 numVectors = Math::divideAndRoundUp(mScalarParameters.size(), 4) + mVectorParameters.size();
		new(members)ConstantBufferStruct::Member(TEXT("Vectors"), TEXT(""), nextMemberOffset, CBMT_FLOAT32, EShaderPrecisionModifier::Half, 1, 4, numVectors, nullptr);
		const uint32 vectorArraySize = numVectors * sizeof(float4);
		nextMemberOffset += vectorArraySize;
		static wstring layoutName(TEXT("MaterialCollection"));
		const uint32 structSize = align(nextMemberOffset, CONSTANT_BUFFER_STRUCT_ALIGNMENT);
		mConstantBufferStruct = makeUniquePtr<ConstantBufferStruct>(
			layoutName,
			TEXT("MaterialCollection"),
			TEXT("MaterialCollection"),
			constructCollectionConstantBufferParameter,
			structSize,
			members,
			false);
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

	void MaterialParameterCollection::updateDefaultResource()
	{
		TArray<float4> parameterData;
		getDefaultParametrData(parameterData);
		mDefaultResource->GameThread_UpdateContents(mStateId, parameterData);
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			updateDefaultResourceCommand,
			Guid, id, mStateId,
			MaterialParameterCollectionInstanceResource*, resource, mDefaultResource,
			{
				GDefaultMaterialParameterCollectionInstances.emplace(id, resource);
			}
		);
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
		updateDefaultResource();
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

	void MaterialParameterCollectionInstanceResource::updateContents(const Guid& inId, const TArray<float4>& indata)
	{
		mConstantBuffer.safeRelease();
		mID = inId;
		if (inId != Guid() && indata.size() > 0)
		{
			mConstantBuffer.safeRelease();
			mConstantBufferLayout.mConstantBufferSize = indata.getTypeSize() * indata.size();
			mConstantBufferLayout.mResourceOffset = 0;
			BOOST_ASSERT(mConstantBufferLayout.mResource.size() == 0);
			mConstantBuffer = RHICreateConstantBuffer(indata.getData(), mConstantBufferLayout, ConstantBuffer_MultiFrame);
		}
	}

	void MaterialParameterCollectionInstanceResource::GameThread_Destroy()
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			DestroyCollectionCommand,
			MaterialParameterCollectionInstanceResource*, resource, this,
			{
				delete resource;
			}
		);
	}

	void MaterialParameterCollectionInstanceResource::GameThread_UpdateContents(const Guid& inId, const TArray<float4>& data)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(
			UpdateCollectionCommand,
			Guid, id, inId,
			TArray<float4>, data, data,
			MaterialParameterCollectionInstanceResource*, resource, this,
			{
				resource->updateContents(id, data);
			}
		);
	}

	DECLARE_SIMPLER_REFLECTION(MaterialParameterCollection);



}