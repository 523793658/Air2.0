#include "D3D11ShaderResource.h"
namespace Air
{
	void D3D11BaseShaderResource::setDirty(bool bInDirty, uint32 currentFrame)
	{
		mDirty = bInDirty;
		if (mDirty)
		{
			mLastFrameWritten = currentFrame;
		}
	}
}