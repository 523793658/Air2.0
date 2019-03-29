#pragma once
#include "Containers/Array.h"
#include "EngineMininal.h"
#include "misc/Optional.h"
namespace Air
{
	class World;
	class Object;
	class AActor;
	class RClass;
	class ActorIteratorState
	{
	public:
		World * mCurrentWorld;
		TArray<Object*> mObjectArray;
		int32 mIndex;

		bool bReachedEnd;
		int32 mConsideredCount;

		AActor* mCurrentActor;
		TArray<AActor*> mSpawnedActorArray;
		RClass* mDesiredClass;
		
		
	};


	template<typename Derived>
	class TActorIteratorBase
	{
	public:
	private:
		TOptional<ActorIteratorState> mState;
	};
}