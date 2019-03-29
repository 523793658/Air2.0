#pragma once
#include "EngineMininal.h"
#include "Classes/Engine/EngineBaseTypes.h"
namespace Air
{
	class TickTaskLevel;
	class Level;
	class World;
	class TickTaskManagerInterface
	{
	public:
		virtual ~TickTaskManagerInterface() {}
		virtual TickTaskLevel* allocateTickTaskLevel() = 0;

		virtual void freeTickTaskLevel(TickTaskLevel* tickTaskLevel) = 0;

		virtual void startFrame(World* inWorld, float deltaSeconds, ELevelTick tickType, const TArray<Level*>& levelsToTick) = 0;

		virtual void runPauseFrame(World* inWorld, float deltaSeconds, ELevelTick tickType, const TArray<Level*>& levelsToTick) = 0;

		virtual void runTickGroup(ETickingGroup group, bool bBlockTillComplete) = 0;

		virtual void endFrame() = 0;

		static ENGINE_API TickTaskManagerInterface& get();
	};
}