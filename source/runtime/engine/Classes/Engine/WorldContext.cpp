#include "Classes/Engine/WorldContext.h"
namespace Air
{
	void WorldContext::setCurrentWorld(World* inWorld)
	{
		if (inWorld != nullptr)
		{
			//inWorld->set
		}

		for (int32 index = 0; index < mExternalReferences.size(); ++index)
		{
			if (mExternalReferences[index] && *mExternalReferences[index] == mThisCurrentWorld)
			{
				*mExternalReferences[index] = inWorld;
			}
		}
		mThisCurrentWorld = inWorld;
	}
}