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

		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			CreateRenderThreadResourceCommand,
			CreateRenderThreadParameters, params, params,
			{
				PrimitiveSceneProxy* sceneProxy = params.mPrimitiveSceneProxy;
		sceneProxy->setTransform(params.mRenderMatrix, params.mWorldBounds, params.mLocalBounds, params.mAttachmentRootPosition);
		sceneProxy->createRenderThreadResources();
			}
		);

		primitive->mAttachmentCounter.increment();
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			AddPrimitiveCommand,
			Scene*, scene, this,
			PrimitiveSceneInfo*, primitiveSceneInfo, primitiveSceneInfo,
			{
				scene->addPrimitiveSceneInfo_RenderThread(RHICmdList, primitiveSceneInfo);
			}
		);
	}

	void Scene::addPrimitiveSceneInfo_RenderThread(RHICommandListImmediate& RHICmdList, PrimitiveSceneInfo* primitiveSceneInfo)
	{
		checkPrimitiveArrays();
		int32 primitiveIndex = mPrimitives.add(primitiveSceneInfo);
		primitiveSceneInfo->mPackedIndex = primitiveIndex;
		mPrimitiveBounds.addUninitialized();
		mPrimitiveVisibilityIds.addUninitialized();
		mPrimitiveOcclusionFlags.addUninitialized();
		mPrimitiveComponentIds.addUninitialized();
		mPrimitiveOcclusionBounds.addUninitialized();

		checkPrimitiveArrays();

		primitiveSceneInfo->linkAttachmentGroup();
		primitiveSceneInfo->linkLODParentComponent();
		primitiveSceneInfo->addToScene(RHICmdList, true);

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

			ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
				UpdateTransformCommand,
				PrimitiveUpdateParams, updateParams, updateParams,
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
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			ClearParameterCollectionsCammand,
			Scene*, scene, this,
			{
				scene->mParameterCollections.empty();
			}
		);
		for (int32 collectionIndex = 0; collectionIndex < inParameterCollections.size(); collectionIndex++)
		{
			ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
				AddParameterCollectionCammand,
				Scene*, scene, this,
				MaterialParameterCollectionInstanceResource*, resource, inParameterCollections[collectionIndex],
				{
					scene->mParameterCollections.emplace(resource->getID(), resource->getConstantBuffer());
				});
		}
	}

	void Scene::setSkyLight(SkyLightSceneProxy* light)
	{
		BOOST_ASSERT(light);
		mNumEnabledSkyLights_GameThread++;

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			SetSkyLightCommand,
			Scene*, scene, this,
			SkyLightSceneProxy*, lightProxy, light,
			{
				BOOST_ASSERT(!scene->mSkyLightStack.contains(lightProxy));
				scene->mSkyLightStack.push(lightProxy);
				const bool bHadSkyLight = scene->mSkyLight != nullptr;
				scene->mSkyLight = lightProxy;
				if (!bHadSkyLight)
				{
					scene->bScenesPrimitivesNeedStaticMeshElementUpdate = true;
				}
			}
		);
	}

	void Scene::disableSkyLight(SkyLightSceneProxy* light)
	{
		BOOST_ASSERT(light);
		mNumEnabledSkyLights_GameThread--;
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			DisableSkyLightCommand,
			Scene*, scene, this,
			SkyLightSceneProxy*, lightProxy, light,
			{
				const bool bHadSkyLight = scene->mSkyLight != nullptr;
				scene->mSkyLightStack.removeSingle(lightProxy);
				if (scene->mSkyLightStack.size() > 0)
				{
					scene->mSkyLight = scene->mSkyLightStack.last();
				}
				else
				{
					scene->mSkyLight = nullptr;
				}
				if ((scene->mSkyLight != nullptr) != bHadSkyLight)
				{
					scene->bScenesPrimitivesNeedStaticMeshElementUpdate = true;
				}
			}
		);
	}
}