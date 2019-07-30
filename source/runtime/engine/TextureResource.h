#pragma once
#include "RenderResource.h"
namespace Air
{

	enum ETextureStreamingState
	{
		TexState_InProgress_Initialization = -1,
		TexState_ReadyFor_Requests = 0,
		TexState_InProgress_Finalization = 1,
		TexState_ReadyFor_Finalization = 2,
		TexState_InProgress_Upload= 3,
		TexState_ReadyFor_Upload= 4,
		TexState_InProgress_Loading = 5,
		TexState_ReadyFor_Loading = 100,
		TexState_InProgress_Allocatioin = 101,
		TexState_InProgress_AsyncAllocatioin = 102,

	};

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

		virtual void initRHI() override;

		virtual void releaseRHI() override;

		void createSamplerState(float mipMapBias);

		int32 getDefaultMipMapBias() const;

	private:
		void getData(uint32 mipIndex, void* dest, uint32 destPitch);

	private:
		const RTexture2D* mOwner;
		Texture2DResourceMem* mResourceMem;
		Texture2DRHIRef mTexture2DRHI;
		int32 mPendingFirstMip;
		int32 mCurrentFirstMap;
		std::unique_ptr<AsyncCreateTextureTask> mAsyncCreateTextureTask;

		void* mMipData[MAX_TEXTURE_MIP_COUNT];

	};
}