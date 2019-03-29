#pragma once
#include "RenderResource.h"
namespace Air
{


	class CreateTextureTask : public NonAbandonableTask
	{
	public:
		struct Arguments 
		{
			uint32 mWidth;
			uint32 mHeight;
			EPixelFormat mFormat;
			uint32 mNumMips;
			uint32 mFlags;
			void** mMipData;
			uint32 mNumNewMips;
			Texture2DRHIRef* mTextureRHI;
			ThreadSafeCounter* mThreadSafeCounter;
		};

		CreateTextureTask(Arguments inArgs)
			:mArgs(inArgs)
		{
			BOOST_ASSERT(mArgs.mTextureRHI);
			BOOST_ASSERT(mArgs.mThreadSafeCounter);

		}
		void doWork();



	private:
		Arguments mArgs;
	};


	typedef AsyncTask<CreateTextureTask> AsyncCreateTextureTask;


	class TextureResource : public Texture
	{
	public:	 
		TextureResource()
		{}
		virtual ~TextureResource() {}

		
	};

	class Texture2DResource : public TextureResource
	{
	public:
		Texture2DResource(class RTexture2D* inOwner, int32 initialMipCount);

		virtual ~Texture2DResource();

	private:
		const RTexture2D* mOwner;
		Texture2DResourceMem* mResourceMem;
		int32 mPendingFirstMip;
		int32 mCurrentFirstMap;
		std::unique_ptr<AsyncCreateTextureTask> mAsyncCreateTextureTask;

		void* mMipData[MAX_TEXTURE_MIP_COUNT];

	};
}