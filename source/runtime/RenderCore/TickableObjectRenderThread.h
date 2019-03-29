#pragma once
#include "RenderCore.h"

namespace Air
{
	class RENDER_CORE_API TickableObjectRenderThread
	{
	public:

		struct RenderThreadTickableObjectsArray : public TArray<TickableObjectRenderThread*>
		{
			~RenderThreadTickableObjectsArray()
			{
				for (int i = 0; i < size(); ++i)
				{
					(*this)[i]->unRegister();
				}
			}
		};

		static RenderThreadTickableObjectsArray mRenderingThreadTickableObjects;

		static RenderThreadTickableObjectsArray
			mRenderingThreadHighFrequencyTickableObjects;

		TickableObjectRenderThread(bool bRegisterImmediately = true, bool inHighRequency = false);

		virtual ~TickableObjectRenderThread();

		void registerTickable(bool bIsRenderingThreadObject = false);

		void unRegister();
	private:
		bool mRegistered;
		bool mHighFrequency;
	};
}