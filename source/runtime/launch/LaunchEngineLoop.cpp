#include "LaunchEngineLoop.h"
#include "HAL/ThreadHeartBeat.h"
#include "HAL/PlatformTLS.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformAffinity.h"
#include "HAL/PlatformMisc.h"
#include "Async/TaskGraphInterfaces.h"
#include "Misc/App.h"
#include "RHI.h"
#include "Framework/Application/SlateApplication.h"
#include "Classes/Materials/MaterialInterface.h"
#include "RenderingThread.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/ISlateRHIRendererModule.h"
#include "MoviePlayer.h"
#include "Classes/Engine/Engine.h"
#include "Classes/Engine/GameEngine.h"
#include "HAL/PlatformProperties.h"
#include "ShaderCompiler.h"
#include "GlobalShader.h"
#include "core/DemoEngine.h"
#include "Misc/CommandLine.h"
#include "Classes/Factories/Factory.h"
namespace Air
{


	EngineLoop::EngineLoop()
	{

	}

	void EngineLoop::tick()
	{
		ThreadHeartBeat::get().heartBeat();

		{
			ENQUEUE_RENDER_COMMAND(
				BeginFrame)([](RHICommandListImmediate& cmdList)
					{
						GRHICommandList.latchBypass();
						GFrameNumberRenderThread++;
						//RHICmdList.pushEvent
						cmdList.beginFrame();
					}
			);
			{
				GEngine->updateTimeAndHandleMaxTickRate();
			}
		}

		//一帧的逻辑
		{
			{
				PlatformMisc::pumpMessages(true);
			}

			bool bIdleMode;
			{
				bIdleMode = shouldUseIdleMode();
				if (bIdleMode)
				{
					PlatformProcess::sleep(.1f);
				}
			}

			if (SlateApplication::isInitialized() && !bIdleMode)
			{
				SlateApplication& slateApp = SlateApplication::get();
				slateApp.pollGameDeviceState();
				slateApp.finishedInputThisFrame();
			}


			GEngine->tick(App::getDeltaTime(), bIdleMode);

			if (SlateApplication::isInitialized())
			{
				SlateApplication::get().tick();
			}

			GFrameCounter++;
			if (GFrameCounter > 6)
			{
				mTotalTickTime += App::getDeltaTime();
			}

			//帧末尾同步
			{
				static FrameEndSync mFrameEndSync;
				mFrameEndSync.sync(false);
			}
		}
	}

	int32 EngineLoop::init()
	{
		
		
		getMoviePlayer()->passLoadingScreenWindowBackToGame();


		initTime();
		GEngine->init(this);

		GEngine->start();

		getMoviePlayer()->waitForMovieToFinish();

		return 0;
	}

	void EngineLoop::clearPendingCleanupObjects()
	{

	}


	int32 EngineLoop::preInit(int32 ArgC, TCHAR* ArgV[], const TCHAR* AdditionalCommandline)
	{


		SlateApplication::create();



		return 0;
	}

	int32 EngineLoop::preInit(const TCHAR* cmdLine)
	{
		if (!CommandLine::set(cmdLine))
		{
			return -1;
		}
		GGameThreadId = PlatformTLS::getCurrentThreadId();
		GIsGameThreadIdInitialized = true;
		bool bHasEditorToken = false;

		GIsClient = true;

		PlatformProcess::setThreadAffinityMask(PlatformAffinity::getMainGameMask());
		//初始化TaskGraph

		TaskGraphInterface::startup(PlatformMisc::numberOfCores());
		TaskGraphInterface::get().attachToThread(ENamedThreads::GameThread);

		if (!loadCoreModules())
		{
			return 1;
		}

		loadPreInitModules();

		if (!appInit())
		{
			return 1;
		}

		if (App::shouldUseThreadingForPerformance() && PlatformMisc::allowRenderThread())
		{
			GUseThreadedRendering = true;
		}
		initializeRenderingCVarsCaching();


		SlateApplication::create();

		RHIInit(bHasEditorToken);

		if (!PlatformProperties::requiresCookedData())
		{
			BOOST_ASSERT(!GShaderCompilingManager);
			GShaderCompilingManager = new ShaderCompilingManager();

		}

		{
			initializeShaderType();
			initializeSharedSamplerStates();
			if (getGlobalShaderMap(GMaxRHIFeatureLevel) == nullptr && GIsRequestingExit)
			{
				return 1;
			}

			MaterialInterface::initDefaultMaterials();
			MaterialInterface::assertDefaultMaterialsExist();
			MaterialInterface::assertDefaultMaterialsPostLoaded();
		}


		if (GUseThreadedRendering)
		{
			if (GRHISupportsRHIThread)
			{
				
			}
			startRenderingThread();
		}

		{
			std::shared_ptr<SlateRenderer> slateRenderer =
				//GUsingNullRHI ? 
				//ModuleManager::get().loadModuleChecked<ISlateRHIRendererModule>("SlateNullRenderer").createsla
				ModuleManager::get().getModuleChecked<ISlateRHIRendererModule>(L"slateRHIRenderer").createSlateRHIRenderer();
			SlateApplication& currentSlateApp = SlateApplication::get();
			currentSlateApp.initializeRenderer(slateRenderer);
			getMoviePlayer()->setSlateRenderer(slateRenderer);

		}

		if (!getMoviePlayer()->isMovieCurrentlyPlaying())
		{
			getMoviePlayer()->initialize();
			//getMoviePlayer()->playMovie();
		}
		if (GEngine == nullptr)
		{
			if (GIsEditor)
			{

			}
			else if (GIsDemo)
			{
				GEngine = newObject<DemoEngine>();
			}
			else
			{
				GEngine = newObject<GameEngine>();
			}
		}

		return 0;
	}


	void EngineLoop::loadPreInitModules()
	{
		ModuleManager::get().loadModule(TEXT("Engine"));
		ModuleManager::get().loadModule(TEXT("Renderer"));




		if (!GUsingNullRHI)
		{
			ModuleManager::get().loadModuleChecked<ISlateRHIRendererModule>(L"slateRHIRenderer");
		}

		ModuleManager::get().loadModule(TEXT("ShaderCore"));

		Factory::initAllFactory();
	}

	bool EngineLoop::loadCoreModules()
	{
		return true;

	}

	bool EngineLoop::loadStartupCoreModules()
	{
		return true;
	}

	bool EngineLoop::loadStartupModule()
	{
		return true;

	}

	void EngineLoop::initTime()
	{

	}

	void EngineLoop::exit()
	{
	}

	bool EngineLoop::shouldUseIdleMode() const
	{
		return false;
	}

	bool EngineLoop::appInit()
	{
		return true;
	}

	void EngineLoop::appPreExit()
	{

	}

	void EngineLoop::appExit()
	{

	}

}