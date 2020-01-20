#include "Classes/Components/ShapeComponent.h"
#include "PrimitiveSceneProxy.h"
#include "SimpleReflection.h"
#include "MeshBatch.h"
#include "DynamicMeshBuild.h"
#include "LocalVertexFactory.h"
namespace Air
{
	void CustomVertexFactory::init_RenderThread(const CustomMeshVertexBuffer* vertexBuffer)
	{
		BOOST_ASSERT(isInRenderingThread());
		DataType newData;
		newData.mPositionComponents = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(vertexBuffer, DynamicMeshVertex, mPosition, VET_Float3);
		newData.mTextureCoordinates.add(VertexStreamComponent(vertexBuffer, STRUCT_OFFSET(DynamicMeshVertex, mTextureCoordinate), sizeof(DynamicMeshVertex), VET_Float2));
		newData.mTangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(vertexBuffer, DynamicMeshVertex, mNormal, VET_PackedNormal);
		newData.mTangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(vertexBuffer, DynamicMeshVertex, mTangent, VET_PackedNormal);
		newData.mColorComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(vertexBuffer, DynamicMeshVertex, mColor, VET_Color);
		setData(newData);
	}


	ShapeComponent::ShapeComponent(const ObjectInitializer& objectInitializer /* = ObjectInitializer::get() */)
		:ParentType(objectInitializer)
	{
		//mMaterial = createDefaultSubObject<RMaterial>(L"material");
	}

	

	class CustomMeshSceneProxy : public PrimitiveSceneProxy
	{
	private:
		std::shared_ptr<MaterialInterface> mMaterial;
		CustomMesh* mMesh;

	public:
		size_t getTypeHash() const override
		{
			static size_t uniquePointer;
			return reinterpret_cast<size_t>(&uniquePointer);
		}


		CustomMeshSceneProxy(const ShapeComponent* inComponent)
			:PrimitiveSceneProxy(inComponent)
			,mMesh(inComponent->mMesh)
		{
			const Color vertexColor(255, 255, 255);
			mMaterial = inComponent->getMaterial(0);
			if (mMaterial == nullptr)
			{
				mMaterial = RMaterial::getDefaultMaterial(MD_Surface);
			}
		}

		void drawStaticElements(StaticPrimitiveDrawInterface* PDI)
		{
			BOOST_ASSERT(isInParallelRenderingThread());
			MeshBatch meshBatch;
			meshBatch.mDynamicVertexData = NULL;
			MeshBatchElement& batchElement = meshBatch.mElements[0];

			const bool bWireframe = false;
			MaterialRenderProxy* materialProxy = nullptr;
			if (bWireframe)
			{

			}
			else
			{
				materialProxy = mMaterial->getRenderProxy(false);
			}


			meshBatch.mVertexFactory = &mMesh->mVertexFactory;
			meshBatch.mMaterialRenderProxy = materialProxy;
			batchElement.mMinVertexIndex = 0;
			batchElement.mMaxVertexIndex = mMesh->mVertexBuffer.mVertices.size() - 1;
			batchElement.mIndexBuffer = &mMesh->mIndexBuffer;
			batchElement.mFirstIndex = 0;
			batchElement.mPrimitiveConstantBuffer = createPrimitiveConstantBufferImmediate(getLocalToWorld(), getBounds(), getLocalBounds(), true, false);
			batchElement.mNumPrimitives = mMesh->mIndexBuffer.mIndices.size() / 3;
			batchElement.mPrimitiveConstantBuffer = createPrimitiveConstantBufferImmediate(getLocalToWorld(), getBounds(), getLocalBounds(), false, false);
			meshBatch.mType = PT_TriangleList;
			meshBatch.bUseDynamicData = false;
			meshBatch.bReverseCulling = false;

			PDI->drawMesh(meshBatch, FLT_MAX);
		}

		PrimitiveViewRelevance getViewRelevance(const SceneView* view) const override
		{
			PrimitiveViewRelevance result;
			result.bStaticRelevance = true;
			result.bDrawRelevance = true;
			result.bRenderInMainPass = shouldRenderInMainPass();
			return result;
		}
	};

	


	PrimitiveSceneProxy* ShapeComponent::createSceneProxy()
	{
		if (!mMesh)
		{
			_genericGeometryData();
		}
		if (!mMesh->bInited)
		{
			mMesh->init();
		}
		return new CustomMeshSceneProxy(this);
	}

	void CustomMesh::init()
	{
		bInited = true;
		mVertexFactory.init(&mVertexBuffer);
		beginInitResource(&mVertexBuffer);
		beginInitResource(&mIndexBuffer);
		beginInitResource(&mVertexFactory);
	}


	DECLARE_SIMPLER_REFLECTION(ShapeComponent);
}