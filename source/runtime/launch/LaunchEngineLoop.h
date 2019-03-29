#pragma once

#include "CoreMinimal.h"
#include "AirEngine.h"
namespace Air
{
	class EngineLoop : public IEngineLoop
	{
	public:
		EngineLoop();

	public:
		int32 preInit(int32 ArgC, TCHAR* ArgV[], const TCHAR* AdditionalCommandline = NULL);

		int32 preInit(const TCHAR* cmdLine);


		void loadPreInitModules();

		bool loadCoreModules();

		bool loadStartupCoreModules();

		bool loadStartupModule();

		virtual int32 init() override;

		void initTime();

		void exit();

		bool shouldUseIdleMode() const;

		virtual void tick() override;

		virtual void clearPendingCleanupObjects() override;

	public:
		static bool appInit();

		static void appPreExit();

		static void appExit();

	private:
		void processLocalPlayerSlateOperations() const;

	protected:
		TArray<float> mFrameTimes;

		double mTotalTickTime;

		double mMaxTickTime;

		uint64 mMaxFrameCounter;

		uint32 mLastFrameCycles;
	};


	extern EngineLoop GEngineLoop;
}