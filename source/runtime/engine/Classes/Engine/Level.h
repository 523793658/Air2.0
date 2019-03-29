#pragma once
#include "EngineMininal.h"
#include "Classes/Engine/EngineBaseTypes.h"
#include "Object.h"
namespace Air
{
	class World;
	class WorldSettings;
	class TickTaskLevel;
	class APlayerController;
	struct LevelCollection;
	struct PendingAutoReceiveInputActor
	{
		AActor* mActor;
		int32 mPlayerIndex;
		PendingAutoReceiveInputActor(AActor* inActor, const int32 inPlayerIndex)
			:mActor(inActor)
			,mPlayerIndex(inPlayerIndex)
		{

		}
	};

	class Level : public Object
	{
		GENERATED_RCLASS_BODY(Level, Object)
	public:
		
		virtual World* getWorld() const override;

		ENGINE_API void initialize(const URL& url);

		ENGINE_API void setWorldSettings(WorldSettings* worldSetting);

		ENGINE_API void sortActorList();


		void incrementalUpdateComponents(int32 numComponentsToUpdate, bool bRerunConstructionScript);

		ENGINE_API void updateLevelComponents(bool bRerunConstructionScripts);

		ENGINE_API void updateModelComponents();

		void pushPenddingAutoReceiveInput(APlayerController* pc);

		void registerActorForAutoReceiveInput(AActor* actor, const int32 playerIndex)
		{
			mPendingAutoReceiveInputActors.add(PendingAutoReceiveInputActor(actor, playerIndex));
		}

		LevelCollection* getCachedLevelCollection() const {
			return mCachedLevelCollection;
		}

		WorldSettings* getWorldSettings(bool bChecked) const;

		void setCachedLevelToCollection(LevelCollection* const inCachedLevelCollection) { mCachedLevelCollection = inCachedLevelCollection; }


		void routeActorInitialize();
	public:

		URL mUrl;
		World* mOwningWorld;
		TArray<AActor*> mActors;
		WorldSettings* mWorldSettings;
		class Model* mModel;
		TickTaskLevel* mTickTaskLevel{ nullptr };

		uint8			bIsVisible : 1;

	private:
		int32			mCurrentActorIndexForUpdateComponents{ 0 };

		TArray<PendingAutoReceiveInputActor> mPendingAutoReceiveInputActors;
		

		uint8 mAreComponentsCurrentlyRegistered : 1;

		LevelCollection* mCachedLevelCollection{ nullptr };
	};
}