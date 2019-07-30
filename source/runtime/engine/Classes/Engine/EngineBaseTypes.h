#pragma once
#include "EngineMininal.h"
#include "Misc/CoreMisc.h"
#include "Containers/EnumAsByte.h"
#include "Async/TaskGraphInterfaces.h"
namespace Air
{
	struct TickContext;

	enum EInputEvent
	{
		IE_Pressed					=0,
		IE_Released					=1,
		IE_Repeat					=2,
		IE_DoubleClick				=3,
		IE_Axis						=4,
		IE_MAX						=5,
	};

	enum ELevelTick
	{
		LEVELTICK_TimeOnly = 0,
		LEVELTICK_ViewportsOnly = 1,
		LEVELTICK_ALL = 2,
		LEVELTICK_PauseTick = 3,
	};

	enum ENetMode
	{
		NM_Standalone,
		NM_DedicatedServer,
		NM_Client,
	};

	enum ETravelType
	{
		TRAVEL_Absolute,
		TRAVEL_Partial,
		TRAVEL_Relative,
		TRAVEL_MAX,
	};

	struct ENGINE_API URL 
	{
		wstring mProtocol;

		wstring mHost;

		int32 mPort;

		wstring mMap;

		TArray<wstring> mOp;

		wstring mPortal;
		int32 mValid;

		wstring toString(bool bFullyQualified = false) const;

		void loadURLConfig(const TCHAR* section, const wstring& filename = GGameIni);

		bool isLocalInternal() const;

		bool isInternal() const;

		void addOption(const TCHAR* str);

		void removeOption(const TCHAR* key, const TCHAR* section = nullptr, const wstring& filename = GGameIni);

		static UrlConfig mUrlConfig;

		static bool bDefaultInitialized;

		URL(const TCHAR* localFilename = nullptr);

		URL(URL* base, const TCHAR* textURL, ETravelType type);

		int32 valid;

		static void staticInit();

		static void staticExit();
	};

	enum ETickingGroup
	{
		TG_PrePhysics,
		TG_StartPhysics,
		TG_DuringPhysics,
		TG_EndPhysics,
		TG_PostPhysics,
		TG_PostUpdateWork,
		TG_LastDemotable,
		TG_NewlySpawned,
		TG_MAX,
	};
	class Object;

	struct TickPrerequisite
	{
		std::shared_ptr<class Object> mPrerequisiteObject;
		struct TickFunction*		mPrerequisiteTickFunction;

		TickPrerequisite()
			:mPrerequisiteTickFunction(nullptr)
		{}

		TickPrerequisite(Object* targetObject, struct TickFunction& targetTickFunction)
			:mPrerequisiteTickFunction(&targetTickFunction),
			mPrerequisiteObject(targetObject)
		{
			
		}

		bool operator == (const TickPrerequisite& other) const
		{
			return mPrerequisiteObject == other.mPrerequisiteObject && mPrerequisiteTickFunction == other.mPrerequisiteTickFunction;
		}

		struct TickFunction* get()
		{
			if (mPrerequisiteObject != nullptr)
			{
				return mPrerequisiteTickFunction;
			}
			return nullptr;
		}
		const struct TickFunction* get() const
		{
			if (mPrerequisiteObject != nullptr)
			{
				return mPrerequisiteTickFunction;
			}
			return nullptr;
		}
	};

	struct ENGINE_API TickFunction
	{
	public:
		TEnumAsByte<enum ETickingGroup> mTickGroup;
		TEnumAsByte<enum ETickingGroup> mEndTickGroup;
	protected:
		TEnumAsByte<enum ETickingGroup> mActualStartTickGroup;
		TEnumAsByte<enum ETickingGroup> mActualEndTickGroup;
	public:
		uint8 bTickEvenWhenPaused : 1;
		uint8 bStartWithTickEnabled : 1;
		uint8 bCanEverTick : 1;
		uint8 bHighPriority : 1;
		uint8 bRunOnAnyThread : 1;
		int32 mTickVisitedGFrameCounter{ 0 };

	private:
		uint8 bRegistered : 1;

		uint8 bWasInterval : 1;
		enum class ETickState : uint8
		{
			Disabled,
			Enabled,
			CoolingDown
		};

		ETickState mTickState;
		int32 mTickQueueGFrameCounter;
		void* mTaskPointer;
		TArray<struct TickPrerequisite> mPrerequisites;

		TickFunction* mNext;
		float mRelativeTickCooldown;
		float mLastTickGameTimeSeconds;

	public:
		float mTickInterval;
		class TickTaskLevel*		mTickTaskLevel;


	public:
		TickFunction();
		~TickFunction();

		void setTickFunctionEnable(bool bInEnable);

		void queuTickFunction(class TickTaskSequencer& tts, const TickContext& tickContext);

		bool isTickFunctionEnabled() const { return mTickState != ETickState::Disabled; }

		bool isTickFunctionRegistered() const { return bRegistered; }

		virtual void executeTick(float deltaTime, ELevelTick tickType, ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent) = 0;
		virtual wstring diagnosticMessage() = 0;

		void registerTickFunction(class Level* level);

		void unRegisterTickFunction();

		GraphEventRef getCompletionHandle() const;

		float calculateDeltaTime(const TickContext& tickContext);

		TArray<struct TickPrerequisite>& getPrerequisites()
		{
			return mPrerequisites;
		}

		void removePrerequisite(Object* targetObject, struct TickFunction& targetTickFunction);

		void addPrerequisite(Object* targetObject, struct TickFunction& targetTickFunction);

		friend class TickTaskSequencer;
		friend class TickTaskManager;
		friend class TickTaskLevel;
		friend class TickFunctionTask;
	};

	struct ActorTickFunction : public TickFunction
	{
		class AActor* mTarget;
		 ENGINE_API virtual void executeTick(float deltaTime, ELevelTick tickType, ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent) override;
		 ENGINE_API virtual wstring diagnosticMessage() override;
	};
	
	struct ActorComponentTickFunction : public TickFunction
	{
		class ActorComponent* mTarget;
		ENGINE_API virtual void executeTick(float deltaTime, ELevelTick tickType, ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent) override;

		ENGINE_API virtual wstring diagnosticMessage() override;

		template<typename ExecuteTickLambda>
		static void ExecuteTickHelper(ActorComponent* target, bool bTickInEditor, float deltaTime, ELevelTick tickType, const ExecuteTickLambda& executeTickFunc);
	};

	enum class EMouseCaptureMode : uint8
	{
		 NoCapture,
		 CapturePermanently,
		 CapturePermanently_IncludingInitialMouseDown,
		 CaptureDuringMouseDown,
		 CaptureDuringRightMouseDown,
	};
}