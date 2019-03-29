#include "TickableObjectRenderThread.h"

namespace Air
{
	TickableObjectRenderThread::TickableObjectRenderThread(bool bRegisterImmediately /* = true */, bool inHighRequency /* = false */)
		:mRegistered(false),
		mHighFrequency(inHighRequency)
	{
		if (bRegisterImmediately)
		{
			registerTickable();
		}
	}

	TickableObjectRenderThread::~TickableObjectRenderThread()
	{
		unRegister();
	}



	void TickableObjectRenderThread::registerTickable(bool bIsRenderingThreadObject /* = false */)
	{
		if (mHighFrequency)
		{
			mRenderingThreadHighFrequencyTickableObjects.push_back(this);
		}
		else
		{
			mRenderingThreadTickableObjects.push_back(this);
		}
		mRegistered = true;
	}

	void TickableObjectRenderThread::unRegister()
	{
		if (mRegistered)
		{
			RenderThreadTickableObjectsArray& tickableObjectArray = mHighFrequency ? mRenderingThreadHighFrequencyTickableObjects : mRenderingThreadTickableObjects;
			auto it = tickableObjectArray.find(this);
			tickableObjectArray.removeAt(it);
			mRegistered = false;
		}
	}
}