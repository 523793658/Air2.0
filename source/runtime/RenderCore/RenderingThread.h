#pragma once
#include "RenderCore.h"
#include "Async/TaskGraphInterfaces.h"
#include "Async/TaskGraph.h"
#include <boost/call_traits.hpp>
#include "TickableObjectRenderThread.h"
#include <mutex>

namespace Air
{
	class DeferredCleanupInterface;

	extern RENDER_CORE_API bool GIsThreadedRendering;

	extern RENDER_CORE_API bool GUseThreadedRendering;

	extern RENDER_CORE_API void startRenderingThread();

	extern RENDER_CORE_API void flushRenderingCommands();

	extern RENDER_CORE_API void advanceFrameRenderPrerequisite();

	extern RENDER_CORE_API void checkRenderingThreadHealth();

	extern RENDER_CORE_API void beginCleanup(DeferredCleanupInterface* cleanupObject);

	class RENDER_CORE_API RenderCommand
	{
	public:
		static ENamedThreads::Type getDesiredThread()
		{
			return ENamedThreads::RenderThread;
		}

		static ESubsequentsMode::Type getSubsequentsMode()
		{
			return ESubsequentsMode::FireAndForget;
		}
	};

	class RENDER_CORE_API DeferredCleanupInterface
	{
	public:
		virtual void finishCleanup() = 0;
		virtual ~DeferredCleanupInterface() {}
	};

	class PendingCleanupObjects
	{
		TArray<DeferredCleanupInterface*> mCleanupArray;
	public:
		PendingCleanupObjects();
		RENDER_CORE_API ~PendingCleanupObjects();
	};


	class RENDER_CORE_API SuspendRenderingThread
	{
	public:
		SuspendRenderingThread(bool bRecreateThread);

		~SuspendRenderingThread();
	private:
		bool bUseRenderingThread;
		bool bWasRenderingThreadRunning;
		bool bRecreateThread;
	};

#define SCOPED_SUSPEND_RENDERING_THREAD(bRecreateThread) SuspendRenderingThread mSuspendRenderingThread(bRecreateThread)


	extern RENDER_CORE_API void checkNotBlockedOnRenderThread();

	extern void gameThreadWaitForTask(const GraphEventRef& task, bool bEmpltyGameThreadTasks = false);

	extern RENDER_CORE_API class RHICommandListImmediate& getImmediateCommandList_ForRenderCommand();


#define shouldExecuteOnRenderThread()	(LIKELY(GIsThreadedRendering || !isInGameThread()))


#define TASK_FUNCTION( Code )	\
	void doTask(ENamedThreads::Type currentThread, const GraphEventRef& myCompletionGraphEvent)	\
	{	\
		RHICommandListImmediate& RHICmdList = getImmediateCommandList_ForRenderCommand();\
		Code;	\
	}

#define TASKNAME_FUNCTION()	\
	FORCEINLINE std::mutex& getMutex()	\
	{		\
		static std::mutex mMutext;	\
		return mMutext;	\
	}



#define ENQUEUE_UNIQUE_RENDER_COMMAND(TypeName, Code)	\
	class EURCMacro_##TypeName : public RenderCommand \
	{	\
	public:	\
		TASKNAME_FUNCTION() \
		TASK_FUNCTION(Code) \
	};	\
	{	\
		if(shouldExecuteOnRenderThread())	\
		{	\
			checkNotBlockedOnRenderThread();	\
			GraphTask<EURCMacro_##TypeName>::createTask().constructAndDispatchWhenReady();	 \
		}	\
		else	\
		{	\
			EURCMacro_##TypeName tempCommand;	\
			tempCommand.doTask(ENamedThreads::GameThread, GraphEventRef());	\
		}	\
	}

#define ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER_DECLARE_OPTTYPENAME(TypeName,ParamType1,ParamName1,ParamValue1,OptTypename,Code) \
	class EURCMacro_##TypeName : public RenderCommand	\
	{	\
	public:	\
		EURCMacro_##TypeName(OptTypename boost::call_traits<ParamType1>::value_type In##ParamName1) :	   \
		ParamName1(In##ParamName1)	\
		{}\
		TASK_FUNCTION(Code)	\
		TASKNAME_FUNCTION()	\
	private:	\
		ParamType1 ParamName1;	\
	};



#define ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER_DECLARE(TypeName,ParamType1,ParamName1,ParamValue1, Code)	  \
	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER_DECLARE_OPTTYPENAME(TypeName,ParamType1,ParamName1,ParamValue1,,Code)


#define ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER_CREATE(TypeName,ParamType1,ParamValue1)  \
	{	\
		if(shouldExecuteOnRenderThread())	\
		{	\
			checkNotBlockedOnRenderThread();	\
			GraphTask<EURCMacro_##TypeName>::createTask().constructAndDispatchWhenReady(ParamValue1);	 \
		}	\
		else \
		{	\
			EURCMacro_##TypeName TempCommand(ParamValue1);	\
			TempCommand.doTask(ENamedThreads::GameThread, GraphEventRef());\
		}	\
	}


#define ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(TypeName, ParamType1, ParamName1, ParamValue1, Code) \
	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER_DECLARE(TypeName, ParamType1, ParamName1, ParamValue1, Code) \
	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER_CREATE(TypeName,ParamType1,ParamValue1)


#define ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER_DECLARE_OPTTYPENAME(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, OptTypename, code)	  \
	class EURCMacro_##TypeName : public RenderCommand	\
	{	\
	public:	\
		EURCMacro_##TypeName(OptTypename boost::call_traits<ParamType1>::value_type in##ParamName1, OptTypename boost::call_traits<ParamType2>::value_type in##ParamName2) :	 \
			ParamName1(in##ParamName1),\
			ParamName2(in##ParamName2)	\
		{}\
		TASK_FUNCTION(code)\
		TASKNAME_FUNCTION(TypeName)	\
	private:	\
		ParamType1	ParamName1;	\
		ParamType2	ParamName2;	\
	};
	


#define ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER_DECLARE(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, code)\
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER_DECLARE_OPTTYPENAME(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, ,code)

#define ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER_CREATE(TypeName, ParamType1, ParamValue1, ParamType2, ParamValue2)	\
	{\
		if(shouldExecuteOnRenderThread())	\
		{	\
			checkNotBlockedOnRenderThread();	\
			GraphTask<EURCMacro_##TypeName>::createTask().constructAndDispatchWhenReady(ParamValue1, ParamValue2);\
		}\
		else \
		{	\
			EURCMacro_##TypeName TempCommand(ParamValue1, ParamValue2);	\
			TempCommand.doTask(ENamedThreads::GameThread, GraphEventRef());  \
		}	\
	}




#define ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, code)	\
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER_DECLARE(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, code)	  \
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER_CREATE(TypeName, ParamType1, ParamValue1, ParamType2, ParamValue2)


#define ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER_DECLARE_OPTTYPENAME(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, ParamType3, ParamName3, ParamValue3, OptTypename, code)	  \
	class EURCMacro_##TypeName : public RenderCommand	\
	{	\
	public:	\
		EURCMacro_##TypeName(OptTypename boost::call_traits<ParamType1>::value_type in##ParamName1, OptTypename boost::call_traits<ParamType2>::value_type in##ParamName2, OptTypename boost::call_traits<ParamType3>::value_type in##ParamName3) :	 \
			ParamName1(in##ParamName1),\
			ParamName2(in##ParamName2),	\
			ParamName3(in##ParamName3)	\
		{}\
		TASK_FUNCTION(code)\
		TASKNAME_FUNCTION(TypeName)	\
	private:	\
		ParamType1	ParamName1;	\
		ParamType2	ParamName2;	\
		ParamType3	ParamName3;	\
	};



#define ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER_DECLARE(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, ParamType3, ParamName3, ParamValue3, code)\
	ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER_DECLARE_OPTTYPENAME(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, ParamType3, ParamName3, ParamValue3, ,code)

#define ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER_CREATE(TypeName, ParamType1, ParamValue1, ParamType2, ParamValue2, ParamType3, ParamValue3)	\
	{\
		if(shouldExecuteOnRenderThread())	\
		{	\
			checkNotBlockedOnRenderThread();	\
			GraphTask<EURCMacro_##TypeName>::createTask().constructAndDispatchWhenReady(ParamValue1, ParamValue2, ParamValue3);\
		}\
		else \
		{	\
			EURCMacro_##TypeName TempCommand(ParamValue1, ParamValue2, ParamValue3);	\
			TempCommand.doTask(ENamedThreads::GameThread, GraphEventRef());  \
		}	\
	}




#define ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, ParamType3, ParamName3, ParamValue3, code)	\
	ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER_DECLARE(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, ParamType3, ParamName3, ParamValue3, code)	  \
	ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER_CREATE(TypeName, ParamType1, ParamValue1, ParamType2, ParamValue2, ParamType3, ParamValue3)








#define ENQUEUE_UNIQUE_RENDER_COMMAND_FIVEPARAMETER_DECLARE_OPTTYPENAME(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, ParamType3, ParamName3, ParamValue3, ParamType4, ParamName4, ParamValue4, ParamType5, ParamName5, ParamValue5, OptTypename, code)	  \
	class EURCMacro_##TypeName : public RenderCommand	\
	{	\
	public:	\
		EURCMacro_##TypeName(OptTypename boost::call_traits<ParamType1>::value_type in##ParamName1, OptTypename boost::call_traits<ParamType2>::value_type in##ParamName2, OptTypename boost::call_traits<ParamType3>::value_type in##ParamName3, OptTypename boost::call_traits<ParamType4>::value_type in##ParamName4, OptTypename boost::call_traits<ParamType5>::value_type in##ParamName5) :	 \
			ParamName1(in##ParamName1),\
			ParamName2(in##ParamName2),	\
			ParamName3(in##ParamName3),	\
			ParamName4(in##ParamName4),	\
			ParamName5(in##ParamName5)	\
		{}\
		TASK_FUNCTION(code)\
		TASKNAME_FUNCTION(TypeName)	\
	private:	\
		ParamType1	ParamName1;	\
		ParamType2	ParamName2;	\
		ParamType3	ParamName3;	\
		ParamType4	ParamName4;	\
		ParamType5	ParamName5;	\
	};



#define ENQUEUE_UNIQUE_RENDER_COMMAND_FIVEPARAMETER_DECLARE(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, ParamType3, ParamName3, ParamValue3, ParamType4, ParamName4, ParamValue4, ParamType5, ParamName5, ParamValue5, code)\
	ENQUEUE_UNIQUE_RENDER_COMMAND_FIVEPARAMETER_DECLARE_OPTTYPENAME(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, ParamType3, ParamName3, ParamValue3, ParamType4, ParamName4, ParamValue4, ParamType5, ParamName5, ParamValue5, ,code)

#define ENQUEUE_UNIQUE_RENDER_COMMAND_FIVEPARAMETER_CREATE(TypeName, ParamType1, ParamValue1, ParamType2, ParamValue2, ParamType3, ParamValue3, ParamType4, ParamValue4, ParamType5, ParamValue5)	\
	{\
		if(shouldExecuteOnRenderThread())	\
		{	\
			checkNotBlockedOnRenderThread();	\
			GraphTask<EURCMacro_##TypeName>::createTask().constructAndDispatchWhenReady(ParamValue1, ParamValue2, ParamValue3, ParamValue4, ParamValue5);\
		}\
		else \
		{	\
			EURCMacro_##TypeName TempCommand(ParamValue1, ParamValue2, ParamValue3, ParamValue4, ParamValue5);	\
			TempCommand.doTask(ENamedThreads::GameThread, GraphEventRef());  \
		}	\
	}




#define ENQUEUE_UNIQUE_RENDER_COMMAND_FIVEPARAMETER(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, ParamType3, ParamName3, ParamValue3, ParamType4, ParamName4, ParamValue4, ParamType5, ParamName5, ParamValue5, code)	\
	ENQUEUE_UNIQUE_RENDER_COMMAND_FIVEPARAMETER_DECLARE(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, ParamType3, ParamName3, ParamValue3, ParamType4, ParamName4, ParamValue4, ParamType5, ParamName5, ParamValue5, code)	  \
	ENQUEUE_UNIQUE_RENDER_COMMAND_FIVEPARAMETER_CREATE(TypeName, ParamType1, ParamValue1, ParamType2, ParamValue2, ParamType3, ParamValue3, ParamType4, ParamValue4, ParamType5, ParamValue5)

#define  ENQUEUE_UNIQUE_RENDER_COMMAND_FIVEPARAMETER_DECLARE_TEMPLATE(TypeName, TemplateParamName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, ParamType3, ParamName3, ParamValue3, ParamType4, ParamName4, ParamValue4, ParamType5, ParamName5, ParamValue5, Code) \
	template <typename TemplateParamName> \
	ENQUEUE_UNIQUE_RENDER_COMMAND_FIVEPARAMETER_DECLARE_OPTTYPENAME(TypeName, ParamType1, ParamName1, ParamValue1, ParamType2, ParamName2, ParamValue2, ParamType3, ParamName3, ParamValue3, ParamType4, ParamName4, ParamValue4, ParamType5, ParamName5, ParamValue5, typename, Code)

#define ENQUEUE_UNIQUE_RENDER_COMMAND_FIVEPARAMETER_CREATE_TEMPLATE(TypeName, TemplateParameterName, ParamType1, ParamValue1, ParamType2, ParamValue2, ParamType3, ParamValue3, ParamType4, ParamValue4, ParamType5, ParamValue5) \
	ENQUEUE_UNIQUE_RENDER_COMMAND_FIVEPARAMETER_CREATE(TypeName<TemplateParameterName>, ParamType1, ParamValue1, ParamType2, ParamValue2, ParamType3, ParamValue3, ParamType4, ParamValue4, ParamType5, ParamValue5)
}
