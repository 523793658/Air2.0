FORCEINLINE int32 getWidth() const
{
	if (mTextureData)
	{
		return mTextureData->mInfo.mWidth;
	}
	return 0;
}

FORCEINLINE int32 getHeight() const
{
	if (mTextureData)
	{
		return mTextureData->mInfo.mHeight;
	}
	return 0;
}

FORCEINLINE int32 getNumMips() const
{
	if (mTextureData)
	{
		return mTextureData->mInfo.mNumMipmaps;
	}
	return 0;
}

FORCEINLINE EPixelFormat getPixelFormat() const
{
	if (mTextureData)
	{
		return mTextureData->mInfo.mFormat;
	}
	return PF_Unknown;
}

FORCEINLINE int32 getMipTailBaseIndex() const
{
	if (mTextureData)
	{
		return Math::max(0, getNumMips() - 1);
	}
	return 0;
}