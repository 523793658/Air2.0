#include "ScenePrivate.h"
#include "PrimitiveSceneInfo.h"
#include "EngineDefines.h"
#include "ParameterCollection.h"
namespace Air
{

	Scene::Scene(World* inWorld, bool bInRequiresHitProxies, bool bInIsEditorScene, bool bCreateFXSystem, ERHIFeatureLevel::Type inFeatureLevel)
		:mWorld(inWorld)
		,mLightOctree(float3::Zero, HALF_WORLD_MAX)
		,mPrimitiveOctree(float3::Zero, HALF_WORLD_MAX)
	{
		mWorld->mScene = this;
	}

	World* Scene::getWorld() const
	{
		return mWorld;
	}

	Scene* Scene::getRenderScene()
	{
		return this;
	}

	void SceneViewState::destroy()
	{

	}

	void SceneViewState::finishCleanup()
	{

	}


	void Scene::addPrimitive(PrimitiveComponent* primitive)
	{
		const float worldTime = getWorld()->getTimeSecondes();
		float deltaTime = worldTime - primitive->mLastSubmitTime;
		if (deltaTime < -0.0001f || primitive->mLastSubmitTime < 0.0001f)
		{
			primitive->mLastSubmitTime = worldTime;
		}
		else if (deltaTime > 0.0001f)
		{
			primitive->mLastSubmitTime = worldTime;
		}
		PrimitiveSceneProxy* primitiveSceneProxy = primitive->createSceneProxy();
		primitive->mSceneProxy = primitiveSceneProxy;
		if (!primitiveSceneProxy)
		{
			return;
		}

		PrimitiveSceneInfo* primitiveSceneInfo = new PrimitiveSceneInfo(primitive, this);

		primitiveSceneProxy->mPrimitiveSceneInfo = primitiveSceneInfo;
		Matrix renderMatrix = primitive->getRenderMatrix();
		float3 attachmentRootPosition(0);
		AActor* attchmentRoot = primitive->getAttachmentRootActor();
		if (attchmentRoot)
		{
			attachmentRootPosition = attchmentRoot->getActorLocation();
		}

		struct CreateRenderThreadParameters
		{
			PrimitiveSceneProxy* mPrimitiveSceneProxy;
			Matrix mRenderMatrix;
			BoxSphereBounds	mWorldBounds;
			float3 mAttachmentRootPosition;
			BoxSphereBounds mLocalBounds;
		};

		CreateRenderThreadParameters params =
		{
			primitiveSceneProxy,
			renderMatrix,
			primitive->mBounds,
			attachmentRootPosition,
			primitive->calcBounds(Transform::identity)
		};

		ENQUEUE_RENDER_COMMAND(
			CreateRenderThreadResourceCommand)([params](RHICommandListImmediate&)
			{
				PrimitiveSceneProxy* sceneProxy = params.mPrimitiveSceneProxy;
		sceneProxy->setTransform(params.mRenderMatrix, params.mWorldBounds, params.mLocalBounds, params.mAttachmentRootPosition);
		sceneProxy->createRenderThreadResources();
			}
		);

		primitive->mAttachmentCounter.increment();
		ENQUEUE_RENDER_COMMAND(
			AddPrimitiveCommand)([this, primitiveSceneInfo](RHICommandListImmediate& RHICmdList)
			{
				this->addPrimitiveSceneInfo_RenderThread(RHICmdList, primitiveSceneInfo);
			}
		);
	}

	template<typename T>
	static void TArraySwapElements(TArray<T>& arr, int i1, int i2)
	{
		T tmp = arr[i1];
		arr[i1] = arr[i2];
		arr[i2] = tmp;
	}

	static void TBitArraySwapElements(TBitArray<>& arr, int32 i1, int32 i2)
	{
		BitReference bitRef1 = arr[i1];
		BitReference bitRef2 = arr[i2];
		bool bit1 = bitRef1;
		bool bit2 = bitRef2;
		bitRef1 = bit2;
		bitRef2 = bit1;
	}


	void Scene::addPrimitiveSceneInfo_RenderThread(RHICommandListImmediate& RHICmdList, PrimitiveSceneInfo* primitiveSceneInfo)
	{
		checkPrimitiveArrays();
		mPrimitives.add(primitiveSceneInfo);
		const Matrix localToWorld = primitiveSceneInfo->mProxy->getLocalToWorld();
		mPrimitiveTransforms.add(localToWorld);
		mPrimitiveSceneProxies.add(primitiveSceneInfo->mProxy);
		mPrimitiveBounds.addUninitialized();
		mPrimitiveFlagsCompact.addUninitialized();
		mPrimitiveVisibilityIds.addUninitialized();
		mPrimitiveOcclusionFlags.addUninitialized();
		mPrimitiveComponentIds.addUninitialized();
		mPrimitiveOcclusionBounds.addUninitialized();
		mPrimitivesNeedingStaticMeshUpdate.add(false);

		const int sourceIndex = mPrimitiveSceneProxies.size() - 1;
		primitiveSceneInfo->mPackedIndex = sourceIndex;

		{
			bool entryFound = false;
			int broadIndex = -1;
			SIZE_T insertProxyHash = primitiveSceneInfo->mProxy->getTypeHash();
			for (broadIndex = mTypeOffsetTable.size() - 1; broadIndex >= 0; broadIndex--)
			{
				if (mTypeOffsetTable[broadIndex].mPrimitiveSceneProxyType == insertProxyHash)
				{
					entryFound = true;
					break;
				}
			}

			if (entryFound == false)
			{
				broadIndex = mTypeOffsetTable.size();
				if (broadIndex)
				{
					TypeOffsetTableEntry entry = mTypeOffsetTable[broadIndex - 1];
					mTypeOffsetTable.push(TypeOffsetTableEntry(insertProxyHash, entry.mOffset));
				}
				else
				{
					mTypeOffsetTable.push(TypeOffsetTableEntry(insertProxyHash, 0));
				}
			}

			while (broadIndex < mTypeOffsetTable.size())
			{
				TypeOffsetTableEntry& nextEntry = mTypeOffsetTable[broadIndex++];
				int destIndex = nextEntry.mOffset++;

				if (destIndex != sourceIndex)
				{
					BOOST_ASSERT(sourceIndex > destIndex);

					mPrimitives[destIndex]->mPackedIndex = sourceIndex;
					mPrimitives[sourceIndex]->mPackedIndex = destIndex;

					TArraySwapElements(mPrimitives, destIndex, sourceIndex);
					TArraySwapElements(mPrimitiveTransforms, destIndex, sourceIndex);
					TArraySwapElements(mPrimitiveSceneProxies, destIndex, sourceIndex);
					TArraySwapElements(mPrimitiveBounds, destIndex, sourceIndex);
					TArraySwapElements(mPrimitiveFlagsCompact, destIndex, sourceIndex);
					TArraySwapElements(mPrimitiveVisibilityIds, destIndex, sourceIndex);
					TArraySwapElements(mPrimitiveOcclusionFlags, destIndex, sourceIndex);
					TArraySwapElements(mPrimitiveComponentIds, destIndex, sourceIndex);
					TArraySwapElements(mPrimitiveOcclusionBounds, destIndex, sourceIndex);
					TBitArraySwapElements(mPrimitivesNeedingStaticMeshUpdate, destIndex, sourceIndex);
					addPrimitiveToUpdateGPU(*this, destIndex);
				}
			}
		}

		checkPrimitiveArrays();

		primitiveSceneInfo->linkAttachmentGroup();

		primitiveSceneInfo->linkLODParentComponent();

		if (GIsEditor)
		{
			primitiveSceneInfo->addToScene(RHICmdList, true);
		}
		else
		{
			const bool bAddToDrawLists = true;
			if (bAddToDrawLists)
			{
				primitiveSceneInfo->addToScene(RHICmdList, true);
			}
			else
			{
				primitiveSceneInfo->addToScene(RHICmdList, true, false);
				primitiveSceneInfo->beginDeferredUpdateStaticMeshes();
			}
		}

		addPrimitiveToUpdateGPU(*this, sourceIndex);
		bPathTracingNeedsInvalidation = true;

	}

	void Scene::checkPrimitiveArrays()
	{
		BOOST_ASSERT(mPrimitives.size() == mPrimitiveBounds.size());
		BOOST_ASSERT(mPrimitives.size() == mPrimitiveVisibilityIds.size());
		BOOST_ASSERT(mPrimitives.size() == mPrimitiveOcclusionFlags.size());
		BOOST_ASSERT(mPrimitives.size() == mPrimitiveComponentIds.size());
		BOOST_ASSERT(mPrimitives.size() == mPrimitiveOcclusionBounds.size());
	}

	void Scene::updatePrimitiveTransform(PrimitiveComponent* primitive)
	{
		const float worldTime = getWorld()->getTimeSecondes();
		float deltaTime = worldTime - primitive->mLastSubmitTime;
		if (deltaTime < -0.0001f || primitive->mLastSubmitTime <= 0.0001f)
		{
			primitive->mLastSubmitTime = worldTime;
		}
		else if(deltaTime > 0.0001f)
		{
			primitive->mLastSubmitTime = worldTime;
		}
		if (primitive->mSceneProxy)
		{
			float3 attachmentRootPosition(0);
			AActor* actor = primitive->getAttachmentRootActor();
			if (actor != nullptr)
			{
				attachmentRootPosition = actor->getActorLocation();
			}
			struct PrimitiveUpdateParams
			{
				Scene* mScene;
				PrimitiveSceneProxy* mPrimitiveSceneProxy;
				BoxSphereBounds mWorldBounds;
				BoxSphereBounds mLocalBounds;
				Matrix	mLocalToWorld;
				float3 mAttachmenentRootPosition;
			};

			PrimitiveUpdateParams updateParams;
			updateParams.mScene = this;
			updateParams.mPrimitiveSceneProxy = primitive->mSceneProxy;
			updateParams.mWorldBounds = primitive->mBounds;
			updateParams.mLocalToWorld = primitive->getRenderMatrix();
			updateParams.mAttachmenentRootPosition = attachmentRootPosition;
			updateParams.mLocalBounds = primitive->calcBounds(Transform::identity);

			ENQUEUE_RENDER_COMMAND(
				UpdateTransformCommand)([updateParams](RHICommandListImmediate& RHICmdList)
				{
					updateParams.mScene->updatePrimitiveTransform_RenderThread(RHICmdList, updateParams.mPrimitiveSceneProxy, updateParams.mWorldBounds, updateParams.mLocalBounds, updateParams.mLocalToWorld, updateParams.mAttachmenentRootPosition);
				}
			);
		}
		else
		{
			addPrimitive(primitive);
		}
	}

	void Scene::updatePrimitiveTransform_RenderThread(RHICommandListImmediate& RHICmdList, PrimitiveSceneProxy* primitiveSceneProxy, const BoxSphereBounds& worldBounds, const BoxSphereBounds& localBounds, const Matrix& localToWorld, const float3& ownerPosition)
	{
		const bool bUpdateStaticDrawLists = !primitiveSceneProxy->staticElementsAlwaysUseProxyPrimitiveUniformBuffer();
		primitiveSceneProxy->getPrimitiveSceneInfo()->removeFromScene(bUpdateStaticDrawLists);

		Scene* scene = (Scene*)&primitiveSceneProxy->getScene();
		primitiveSceneProxy->setTransform(localToWorld, worldBounds, localBounds, ownerPosition);
		primitiveSceneProxy->getPrimitiveSceneInfo()->addToScene(RHICmdList, bUpdateStaticDrawLists);
	}
	
	void Scene::updateParameterCollections(const TArray<class MaterialParameterCollectionInstanceResource *>& inParameterCollections)
	{
		ENQUEUE_RENDER_COMMAND(
			ClearParameterCollectionsCammand)([this](RHICommandListImmediate&)
			{
				this->mParameterCollections.empty();
			}
		);
		for (int32 collectionIndex = 0; collectionIndex < inParameterCollections.size(); collectionIndex++)
		{
			MaterialParameterCollectionInstanceResource* resource = inParameterCollections[collectionIndex];

			ENQUEUE_RENDER_COMMAND(
				AddParameterCollectionCammand)([this, resource](RHICommandListImmediate&)
				{
					this->mParameterCollections.emplace(resource->getID(), resource->getConstantBuffer());
				});
		}
	}

	void Scene::setSkyLight(SkyLightSceneProxy* light)
	{
		BOOST_ASSERT(light);
		mNumEnabledSkyLights_GameThread++;

		ENQUEUE_RENDER_COMMAND(
			SetSkyLightCommand)([this, light](RHICommandListImmediate& RHICmdList)
			{
				BOOST_ASSERT(!this->mSkyLightStack.contains(light));
				this->mSkyLightStack.push(light);
				const bool bHadSkyLight = this->mSkyLight != nullptr;
				this->mSkyLight = light;
				if (!bHadSkyLight)
				{
					this->bScenesPrimitivesNeedStaticMeshElementUpdate = true;
				}
			}
		);
	}

	void Scene::disableSkyLight(SkyLightSceneProxy* light)
	{
		BOOST_ASSERT(light);
		mNumEnabledSkyLights_GameThread--;
		ENQUEUE_RENDER_COMMAND(
			DisableSkyLightCommand)([this, light](RHICommandListImmediate& RHICmdList)
			{
				const bool bHadSkyLight = this->mSkyLight != nullptr;
				this->mSkyLightStack.removeSingle(light);
				if (this->mSkyLightStack.size() > 0)
				{
					this->mSkyLight = this->mSkyLightStack.last();
				}
				else
				{
					this->mSkyLight = nullptr;
				}
				if ((this->mSkyLight != nullptr) != bHadSkyLight)
				{
					this->bScenesPrimitivesNeedStaticMeshElementUpdate = true;
				}
			}
		);
	}

	bool PersistentConstantBuffers::updateViewConstantBuffer(const ViewInfo& view)
	{
		if (mCachedView != &view)
		{
			mViewConstantBuffer.updateConstantBufferImmediate(*view.mCachedViewConstantShaderParameters);
			if ((view.isInstancedStereoPass() || view.bIsMobileMultiViewEnable) && view.mFamily->mViews.size() > 0)
			{
				const ViewInfo& instancedView = getInstancedView(view);
				mInstancedViewConstantBuffer.updateConstantBufferImmediate(reinterpret_cast<InstancedViewConstantShaderParameters&>(*instancedView.mCachedViewConstantShaderParameters));
			}
			else
			{
				mInstancedViewConstantBuffer.updateConstantBufferImmediate(reinterpret_cast<InstancedViewConstantShaderParameters&>(*view.mCachedViewConstantShaderParameters));
			}
			mCachedView = &view;
			return true;
		}
		return false;
	}
}