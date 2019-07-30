#pragma once
#include "EngineMininal.h"
#include "Classes/Engine/EngineType.h"
namespace Air
{
	class ViewportClient;
	class World;
	struct WorldContext 
	{
		std::shared_ptr<ViewportClient> mGameViewport;

		void addRef(World*& worldPtr)
		{
			worldPtr = mThisCurrentWorld;
			mExternalReferences.addUnique(&worldPtr);
		}

		FORCEINLINE World* getWorld() const
		{
			return mThisCurrentWorld;
		}

		void setCurrentWorld(std::shared_ptr<World>& inWorld);

		TArray<World**> mExternalReferences;
	public:
		World* mThisCurrentWorld;

		EWorldType::Type mWorldType;

		wstring mContextHandle;

		std::shared_ptr<class GameInstance> mOwningGameInstance;
	};
}