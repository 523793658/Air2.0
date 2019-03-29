#include "ConvexVolume.h"
namespace Air
{
	void ConvexVolume::init()
	{
		int32 numToAdd = mPlanes.size() / 4;
		int32 numRemaining = mPlanes.size() % 4;
		mPermutedPlanes.reserve(numToAdd * 4 + (numRemaining ? 4 : 0));
		for (int32 count = 0, offset = 0; count < numToAdd; count++, offset += 4)
		{
			new(mPermutedPlanes)Plane(mPlanes[offset + 0].x, mPlanes[offset + 1].x, mPlanes[offset + 2].x, mPlanes[offset + 3].x);

			new(mPermutedPlanes)Plane(mPlanes[offset + 0].y, mPlanes[offset + 1].y, mPlanes[offset + 2].y, mPlanes[offset + 3].y);

			new(mPermutedPlanes)Plane(mPlanes[offset + 0].z, mPlanes[offset + 1].z, mPlanes[offset + 2].z, mPlanes[offset + 3].z);

			new(mPermutedPlanes)Plane(mPlanes[offset + 0].w, mPlanes[offset + 1].w, mPlanes[offset + 2].w, mPlanes[offset + 3].w);
		}
		if (numRemaining)
		{
			Plane last1, last2, last3, last4;
			switch (numRemaining)
			{
			case 3:
			{
				last1 = mPlanes[numToAdd * 4 + 0];
				last2 = mPlanes[numToAdd * 4 + 1];
				last3 = mPlanes[numToAdd * 4 + 2];
				last4 = last1;
				break;
			}
			case 2:
			{
				last1 = mPlanes[numToAdd * 4 + 0];
				last2 = mPlanes[numToAdd * 4 + 1];
				last3 = last4 = last1;
				break;
			}
			case 1:
			{
				last2 = last3 = last4 = last1 = mPlanes[numToAdd * 4 + 0];
				break;
			}
			default:
			{
				last2 = last3 = last4 = last1 = Plane(0, 0, 0, 0);
			}
				break;
			}
			new(mPermutedPlanes)Plane(last1.x, last2.x, last3.x, last4.x);

			new(mPermutedPlanes)Plane(last1.y, last2.y, last3.y, last4.y);
			new(mPermutedPlanes)Plane(last1.z, last2.z, last3.z, last4.z);
			new(mPermutedPlanes)Plane(last1.w, last2.w, last3.w, last4.w);
		}
	}

	void getViewFrustumBounds(ConvexVolume& outResult, const Matrix& viewProjectionMatrix, const Plane& inFarPlane, bool boverredeFarPlane, bool bUseNearPlane)
	{
		outResult.mPlanes.reserve(6);
		Plane temp;
		if (bUseNearPlane && viewProjectionMatrix.getFrustumNearPlane(temp))
		{
			outResult.mPlanes.push_back(temp);
		}

		if (viewProjectionMatrix.getFrustumLeftPlane(temp))
		{
			outResult.mPlanes.push_back(temp);
		}
		if (viewProjectionMatrix.getFrustumRightPlane(temp))
		{
			outResult.mPlanes.push_back(temp);
		}
		if (viewProjectionMatrix.getFrustumTopPlane(temp))
		{
			outResult.mPlanes.push_back(temp);
		}
		if (viewProjectionMatrix.getFrustumBottomPlane(temp))
		{
			outResult.mPlanes.push_back(temp);
		}
		if (boverredeFarPlane)
		{
			outResult.mPlanes.push_back(inFarPlane);
		}
		else if (viewProjectionMatrix.getFrustumFarPlane(temp))
		{
			outResult.mPlanes.push_back(temp);
		}
		outResult.init();
	}

	void getViewFrustumBounds(ConvexVolume& outResult, const Matrix& viewProjectMatrix, bool bUseNearPlane)
	{
		getViewFrustumBounds(outResult, viewProjectMatrix, Plane(), false, bUseNearPlane);
	}
}