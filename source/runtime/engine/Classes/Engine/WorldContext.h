#pragma once
#include "EngineMininal.h"
#include "Classes/Engine/EngineType.h"
namespace Air
{
	class ViewportClient;
	class World;
	struct WorldContext 
	{
		ViewportClient* mGameViewport;

		void addRef(World*& worldPtr)
		{
			worldPtr = mThisCurrentWorld;
			mExternalReferences.addUnique(&worldPtr);
		}

		FORCEINLINE World* getWorld() const
		{
			return mThisCurrentWorld;
		}

		void setCurrentWorld(World* inWorld);

		TArray<World**> mExternalReferences;
	public:
		World* mThisCurrentWorld;

		EWorldType::Type mWorldType;

		wstring mContextHandle;

		class GameInstance* mOwningGameInstance{ nullptr };
	};
}