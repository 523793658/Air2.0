#include "ResImporter/TextureImporter.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Texture2D.h"
#include "TextureCube.h"
namespace Air
{
	TextureImporter::TextureImporter(TCHAR* extension)
		:Importer(extension)
	{
		
	}

	

	class TextureLoadingDesc : public ResLoadingDesc
	{
	private:
		struct TextureDesc
		{
			std::wstring mResPath;
			uint32_t mAccessHint;
			TextureImporter* mImporter;
			
			std::shared_ptr<EngineResourceData> mTexData;

			std::shared_ptr<std::shared_ptr<RTexture>> mResource;
		};

	public:
		explicit TextureLoadingDesc(std::wstring const & resPath, uint32 accessHint)
		{
			mTexDesc.mResPath = resPath;
			mTexDesc.mAccessHint = accessHint;
			mTexDesc.mTexData = MakeSharedPtr<TextureData>();
			mTexDesc.mResource = MakeSharedPtr<std::shared_ptr<RTexture>>();
			mTexDesc.mImporter = dynamic_cast<TextureImporter*>(ImporterMamanger::get()->getImporter(Paths::getExtension(resPath).c_str()));
			mResourceClass = RTexture::StaticClass();
			BOOST_ASSERT(mTexDesc.mImporter);
		}

		uint64_t type() const override
		{
			static uint64_t const mType = boost::hash_value("TextureLoadingDesc");
			return mType;
		}

		bool stateLess() const override
		{
			return true;
		}

		void copyDataFrom(ResLoadingDesc const & rhs)
		{
			BOOST_ASSERT(this->type() == rhs.type());
			TextureLoadingDesc const & tld = static_cast<TextureLoadingDesc const &>(rhs);
			mTexDesc.mResPath = tld.mTexDesc.mResPath;
			mTexDesc.mAccessHint = tld.mTexDesc.mAccessHint;
			mTexDesc.mImporter = tld.mTexDesc.mImporter;
			mTexDesc.mResource = tld.mTexDesc.mResource;
			mTexDesc.mTexData = tld.mTexDesc.mTexData;
		}

		void setResource(std::shared_ptr<Object> ptr) override
		{
			*mTexDesc.mResource = std::static_pointer_cast<RTexture>(ptr);
		}

		std::shared_ptr<void> cloneResourceFrom(std::shared_ptr<void> const & resource)
		{
			return resource;
		}

		bool match(ResLoadingDesc const & rhs) const override
		{
			if (this->type() == rhs.type())
			{
				TextureLoadingDesc const & tld = static_cast<TextureLoadingDesc const&>(rhs);
				return mTexDesc.mResPath == tld.mTexDesc.mResPath && mTexDesc.mAccessHint == tld.mTexDesc.mAccessHint;
			}
			return false;
		}

		

		bool hasSubThreadStage() const override
		{
			return true;
		}

		void subThreadStage() override
		{
			std::lock_guard<std::mutex> lock(mMainThreadStageMutex);

			std::shared_ptr<RTexture> const & tex = *mTexDesc.mResource;
			if (tex && mTexDesc.mTexData->bLoaded)
			{
				return;
			}
			mTexDesc.mImporter->decode(mTexDesc.mResPath, mTexDesc.mTexData);
		}

		void mainThreadStageNoLock()
		{
			(*mTexDesc.mResource)->conditionalPostLoad();
		}

		void mainThreadStage() override
		{
			std::lock_guard<std::mutex> lock(mMainThreadStageMutex);
			mainThreadStageNoLock();
		}

		std::shared_ptr<RTexture> createTexture()
		{
			
		}

		virtual std::shared_ptr<Object> createResource() override
		{
			std::shared_ptr<TextureData> texData = std::dynamic_pointer_cast<TextureData>(mTexDesc.mTexData);

			{
				Archive* reader = IFileManager::get().createFileReader(mTexDesc.mResPath.c_str());
				texData->mInfo = mTexDesc.mImporter->getTextureInfo(*reader);
				delete reader;
			}

			uint32 array_size = texData->mInfo.mArraySize;
			if (RTexture::TT_Cube == texData->mInfo.mType)
			{
				array_size *= 6;
			}

			switch (texData->mInfo.mType)
			{
			case RTexture::TT_2D:
			{
				mResourceClass = RTexture2D::StaticClass();
			}
			break;
			case RTexture::TT_Cube:
			{
				mResourceClass = RTextureCube::StaticClass();
			}
			break;
			default:
				break;
			}
			std::shared_ptr<RTexture> ptr = std::dynamic_pointer_cast<RTexture>(ResLoadingDesc::createResource());
			ptr->mTextureData = std::dynamic_pointer_cast<TextureData>(mTexDesc.mTexData);
			return ptr;
		}

		virtual wstring name() const override
		{
			return mTexDesc.mResPath;
		}

		std::shared_ptr<Object> resource() const override
		{
			return *mTexDesc.mResource;
		}
	private:
		TextureDesc mTexDesc;
		std::mutex mMainThreadStageMutex;
	};

	std::shared_ptr<ResLoadingDesc> RTexture::createLoadingDesc(wstring const & path)
	{
		return MakeSharedPtr<TextureLoadingDesc>(path, 0);
	}

	bool TextureData::tryLoadMips(int32 firstMipToLoad, void** outMipData)
	{
		switch (mInfo.mType)
		{
		case RTexture::TT_2D:
			return tryLoadMips_TT_2D(firstMipToLoad, outMipData);
		default:
			break;
		}
		return false;
	}

	bool TextureData::tryLoadMips_TT_2D(int32 firstMipToLoad, void** outMipData)
	{
		int32 numMipsCached = 0;
		for (int32 mipIndex = firstMipToLoad; mipIndex < mInfo.mNumMipmaps; mipIndex++)
		{
			ElementInitData& mip = mInitData[mipIndex];
			if (mip.mSlicePitch > 0)
			{
				if (outMipData)
				{
					outMipData[mipIndex - firstMipToLoad] = Memory::malloc(mip.mSlicePitch);
					Memory::memcpy(outMipData[mipIndex - firstMipToLoad], mip.mData, mip.mSlicePitch);
				}
				numMipsCached++;
			}
		}
		if (numMipsCached != (mInfo.mNumMipmaps - firstMipToLoad))
		{
			for (int32 mipIndex = firstMipToLoad; mipIndex < mInfo.mNumMipmaps; ++mipIndex)
			{
				Memory::free(outMipData[mipIndex - firstMipToLoad]);
				outMipData[mipIndex - firstMipToLoad] = nullptr;
			}
			return false;
		}
		return true;
	}

}