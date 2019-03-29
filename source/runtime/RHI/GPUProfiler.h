#pragma once
#include "RHIConfig.h"
#include "Template/RefCounting.h"
namespace Air
{
	class GPUProfilerEventNodeStats : public RefCountedObject
	{
	public:
		uint32 mNumDraws{ 0 };
		uint32 mNumPrimitives{ 0 };
		uint32 mNumVertices{ 0 };
		uint32 mNumTotalDraws{ 0 };
		uint32 mNumTotalPrimitives{ 0 };
		uint32 mNumTotalVertices{ 0 };
		float mTimingResult{ 0 };
		uint32 mNumEvents{ 0 };

		const GPUProfilerEventNodeStats operator +=(const GPUProfilerEventNodeStats& rhs)
		{
			mNumDraws += rhs.mNumDraws;
			mNumPrimitives += rhs.mNumPrimitives;
			mNumVertices += rhs.mNumVertices;
			mNumTotalDraws += rhs.mNumDraws;
			mNumTotalPrimitives += rhs.mNumPrimitives;
			mNumTotalVertices += rhs.mNumVertices;
			mTimingResult += rhs.mTimingResult;
			mNumEvents += rhs.mNumEvents;
			return *this;
		}
	};


	class GPUProfilerEventNode : public GPUProfilerEventNodeStats
	{
	public:
		GPUProfilerEventNode(const TCHAR* inName, GPUProfilerEventNode* inParent):
			mName(inName),
			mParent(inParent)
		{}

		~GPUProfilerEventNode(){}
		wstring mName;
		GPUProfilerEventNode* mParent;
		TArray<TRefCountPtr<GPUProfilerEventNode>> mChildren;

		virtual float getTiming() { return 0.0f; }
		virtual void startTiming() {}
		virtual void stopTiming(){}
	};

	struct RHI_API GPUProfilerEventNodeFrame
	{
		virtual ~GPUProfilerEventNodeFrame(){}
		TArray<TRefCountPtr<GPUProfilerEventNode>> mEventTree;

		virtual void startFrame() {}
		virtual void endFrame() {}
		void dumpEventTree();
		virtual float getRootTimingResults() { return 0.0f; }
		virtual void logDisjointQuery(){}

		virtual bool platformDisablesVSync() const { return false; }
	};

	struct RHI_API GPUTiming 
	{
	public:
		static bool isSupported()
		{
			return GIsSupported;
		}

		static uint64 getTimingFrequency()
		{
			return GTimingFrequency;
		}

		typedef void (PlatformStaticInitialize)(void*);

		static void staticInitialize(void* userData, PlatformStaticInitialize* platformFunction)
		{
			if (!GAreGlobalsInitialized && platformFunction)
			{
				(*platformFunction)(userData);
				if (GTimingFrequency != 0)
				{
					GIsSupported = true;
				}
				else
				{
					GIsSupported = false;
				}
				GAreGlobalsInitialized = true;
			}
		}



	protected:
		static bool GAreGlobalsInitialized;
		static bool GIsSupported;
		static uint64 GTimingFrequency;
	};


	struct RHI_API GPUProfiler
	{

	};

}