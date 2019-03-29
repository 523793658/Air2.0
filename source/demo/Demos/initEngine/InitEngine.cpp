#include "Demos/initEngine/InitEngine.h"
#include "Classes/Engine/StaticMeshActor.h"
#include "ApplicationManager.h"
#include "SimpleShapeActor.h"
#include "Classes/GameFramework/PlayerController.h"
#include "Classes/Engine/DirectionalLight.h"
#include "Classes/GameFramework/SpectatorPawn.h"
#include "Texture.h"
#include "Classes/Materials/Material.h"
#include "Classes/Materials/MaterialInstanceDynamic.h"
namespace Demo
{
	using namespace Air;



	DemoInitEngine::DemoInitEngine(const Air::ObjectInitializer& objectInitalizer)
	{
		OnKeyPress = std::bind(&DemoInitEngine::onKeyPress, this, std::placeholders::_1);
		OnkeyRelease = std::bind(&DemoInitEngine::onKeyRelease, this, std::placeholders::_1);
		OnMousePress = std::bind(&DemoInitEngine::onMousePress, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		OnMouseReleas = std::bind(&DemoInitEngine::onMouseRelease, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		OnMouseMove = std::bind(&DemoInitEngine::onMouseMove, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	}

	//DECALRE_DEMO(DemoInitEngine);
	void DemoInitEngine::start()
	{
		LocalPlayer* player = mEngine->getFirstGamePlayer(mWorld);
		player->mPlayerComtroller->getPawnOrSpectator()->setActorLocationAndRotation(float3(0, 5, -5), Rotator::ZeroRotator);
		player->mPlayerComtroller->setControlRotation(Rotator(45, 0, 0));

		ActorSpawnParameters spawnInfo;


		CubeActor *actor = mWorld->spawnActor<CubeActor>(spawnInfo);

		ADirectionalLight* light = mWorld->spawnActor<ADirectionalLight>(spawnInfo);

		light->setLightColor(LinearColor(1.0f, 1.0f, 1.0f, 1.0f));
		
		auto tex = loadObjectAsync<RTexture>(TEXT("assets/textures/uffizi_cross.dds"));
		mWorld->setSkyTexture(tex);

		RMaterial* material = RMaterial::getDefaultMaterial(MD_Surface);

		PrimitiveComponent* component = actor->findComponentByClass<PrimitiveComponent>();
		auto materialInstance = RMaterialInstanceDynamic::create(material, component);

		materialInstance->setScalarParameterValue(TEXT("BaseColor"), 0.1f);
		component->setMaterial(0, materialInstance);
	}

	void DemoInitEngine::init(DemoEngine* inEngine)
	{
		Application::init(inEngine);

		mFreeCamera = new FreeCamera(*mCamera);

		registerEvent(*inEngine);
	}
	void DemoInitEngine::update(float deltaSeconds, bool bIdleMode)
	{
		mFreeCamera->update(deltaSeconds, bIdleMode);
	}

	void DemoInitEngine::registerEvent(DemoEngine& engine)
	{
		
	}

	void DemoInitEngine::onKeyPress(uint32 key)
	{
		mFreeCamera->onKeyPress(key);
	}

	void DemoInitEngine::onKeyRelease(uint32 key)
	{
		mFreeCamera->onKeyRelease(key);
	}

	void DemoInitEngine::onMousePress(uint32 key, int32 x, int32 y)
	{
		mFreeCamera->onMousePress(key, x, y);
	}

	void DemoInitEngine::onMouseRelease(uint32 key, int32 x, int32 y)
	{
		mFreeCamera->onMouseRelease(key, x, y);
	}

	void DemoInitEngine::onMouseMove(uint32 key, int32 x, int32 y)
	{
		mFreeCamera->onMouseMove(key, x, y);
	}

	void FreeCamera::update(float deltaTime, bool bIdleMode)
	{
		if (bIsMoving)
		{
			float3 location = mCamera.getLocation();
			auto quat = mCamera.getRotation().quaternion();
			float3 sp = mSpeed * deltaTime;
			location += quat.getAxisX() * sp.x + quat.getAxisY() * sp.y + quat.getAxisZ() * sp.z;
			mCamera.setLocation(location);
		}
	}

	void FreeCamera::onKeyPress(uint32 key)
	{
		if (key == KEY_W)
		{
			mSpeed.z += 1.0f;
		}
		if (key == KEY_S)
		{
			mSpeed.z -= 1.0f;
		}
		if (key == KEY_A)
		{
			mSpeed.x -= 1.0f;
		}
		if (key == KEY_D)
		{
			mSpeed.x += 1.0f;
		}
		if (key == KEY_Q)
		{
			mSpeed.y -= 1.0f;
		}
		if (key == KEY_E)
		{
			mSpeed.y += 1.0f;
		}
	}

	void FreeCamera::onKeyRelease(uint32 key)
	{
		if (key == KEY_W)
		{
			mSpeed.z -= 1.0f;
		}
		if (key == KEY_S)
		{
			mSpeed.z += 1.0f;
		}
		if (key == KEY_A)
		{
			mSpeed.x += 1.0f;
		}
		if (key == KEY_D)
		{
			mSpeed.x -= 1.0f;
		}
		if (key == KEY_Q)
		{
			mSpeed.y += 1.0f;
		}
		if (key == KEY_E)
		{
			mSpeed.y -= 1.0f;
		}
	}
	void FreeCamera::onMousePress(uint32 key, int32 x, int32 y)
	{
		if (key == MOUSE_RIGHT_BUTTON)
		{
			bIsRotating = true;
			bIsMoving = true;
			mLastMousePosition.x = x;
			mLastMousePosition.y = y;
		}
	}

	void FreeCamera::onMouseRelease(uint32 key, int32 x, int32 y)
	{
		if (key == MOUSE_RIGHT_BUTTON)
		{
			bIsRotating = false;
			bIsMoving = false;
		}
	}

	void FreeCamera::onMouseMove(uint32 key, int32 x, int32 y)
	{
		if (bIsRotating)
		{
			Rotator rotator = mCamera.getRotation();
			rotator.mYaw += (x - mLastMousePosition.x) * 0.2f;
			rotator.mPitch += (y - mLastMousePosition.y) * 0.2f;
			mCamera.setRotation(rotator);


			mLastMousePosition.x = x;
			mLastMousePosition.y = y;
		}

		

	}


	DECALRE_DEMO(DemoInitEngine);
}