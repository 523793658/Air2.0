#include "BoundShaderStateCache.h"
#include "HAL/CriticalSection.h"
#include "Misc/ScopeLock.h"
namespace Air
{

	typedef TMap<BoundShaderStateKey, CachedBoundShaderStateLink*> BoundShaderStateCache;

	typedef TMap<BoundShaderStateKey, CachedBoundShaderStateLink_ThreadSafe*> BoundShaderStateCache_ThreadSafe;

	static BoundShaderStateCache GBoundShaderStateCache;
	static BoundShaderStateCache_ThreadSafe GBoundShaderStateCache_ThreadSafe;

	static BoundShaderStateCache_ThreadSafe& getBoundShaderStateCache_ThreadSafe()
	{
		return GBoundShaderStateCache_ThreadSafe;
	}

	static BoundShaderStateCache& getBoundShaderStateCache()
	{
		return GBoundShaderStateCache;
	}

	static CriticalSection BoundShaderStateCacheLock;



	CachedBoundShaderStateLink::CachedBoundShaderStateLink(RHIVertexDeclaration* vertexDeclaration, RHIVertexShader* vertexShader, RHIPixelShader* pixelShader, RHIHullShader* hullShader, RHIDomainShader* domainShader, RHIGeometryShader* geometryShader, RHIBoundShaderState* inBoundShaderState, bool bAddToSingleThreadedCached /* = true */)
		:mBoundShaderState(inBoundShaderState),
		mKey(vertexDeclaration, vertexShader, pixelShader),
		bAddedToSingleThreadedCache(bAddToSingleThreadedCached)
	{
		if (bAddToSingleThreadedCached)
		{
			getBoundShaderStateCache().emplace(mKey, this);
		}
	}

	CachedBoundShaderStateLink::CachedBoundShaderStateLink(RHIVertexDeclaration* vertexDeclaration, RHIVertexShader* vertexShader, RHIPixelShader* pixelShader, RHIBoundShaderState* inBoundShaderState, bool bAddToSingleThreadedCache /* = true */)
		:mBoundShaderState(inBoundShaderState),
		mKey(vertexDeclaration, vertexShader, pixelShader),
		bAddedToSingleThreadedCache(bAddToSingleThreadedCache)
	{
		if (bAddToSingleThreadedCache)
		{
			getBoundShaderStateCache().emplace(mKey, this);
		}
	}

	CachedBoundShaderStateLink::~CachedBoundShaderStateLink()
	{
		if (bAddedToSingleThreadedCache)
		{
			getBoundShaderStateCache().erase(mKey);
			bAddedToSingleThreadedCache = false;
		}
	}

	CachedBoundShaderStateLink* getCachedBoundShaderState(
		RHIVertexDeclaration* vertexDeclaration,
		RHIVertexShader* vertexShader,
		RHIPixelShader* pixelShader,
		RHIHullShader* hullShader,
		RHIDomainShader* domainShader,
		RHIGeometryShader* geometryShader
	)
	{
		auto it = getBoundShaderStateCache().find(BoundShaderStateKey(vertexDeclaration, vertexShader, pixelShader, hullShader, domainShader, geometryShader));
		if (it != getBoundShaderStateCache().end())
		{
			return it->second;
		}
		return nullptr;
	}

	void CachedBoundShaderStateLink_ThreadSafe::addToCache()
	{
		ScopeLock lock(&BoundShaderStateCacheLock);
		getBoundShaderStateCache_ThreadSafe().emplace(mKey, this);
	}

	void CachedBoundShaderStateLink_ThreadSafe::removeFromCache()
	{
		ScopeLock lock(&BoundShaderStateCacheLock);
		getBoundShaderStateCache_ThreadSafe().erase(mKey);
	}

	BoundShaderStateRHIRef getCachedBoundShaderState_ThreadSafe(RHIVertexDeclaration* vertexDeclaration,
		RHIVertexShader* vertexShader,
		RHIPixelShader* pixelShader,
		RHIHullShader* hullShader,
		RHIDomainShader* domainShader,
		RHIGeometryShader* geometryShader)
	{
		ScopeLock lock(&BoundShaderStateCacheLock);
		auto it = getBoundShaderStateCache_ThreadSafe().find(BoundShaderStateKey(vertexDeclaration, vertexShader, pixelShader, hullShader, domainShader, geometryShader));
		if (it != getBoundShaderStateCache_ThreadSafe().end())
		{
			return it->second->mBoundShaderState;
		}
		return BoundShaderStateRHIRef();
	}

	void emptyCachedBoundShaderStates()
	{
		getBoundShaderStateCache().clear();
		getBoundShaderStateCache_ThreadSafe().clear();
	}
}