#pragma once
#include "RHIConfig.h"
#include "RHIResource.h"
namespace Air
{

	class RHIVertexDeclaration;
	class RHIVertexShader;
	class RHIPixelShader;
	class RHIHullShader;
	class RHIDomainShader;
	class RHIGeometryShader;

	class BoundShaderStateKey
	{
	public:
		BoundShaderStateKey(
			RHIVertexDeclaration* inVertexDeclaration,
			RHIVertexShader* inVertexShader,
			RHIPixelShader* inPixelShader,
			RHIHullShader* inHullShader = nullptr,
			RHIDomainShader* inDomainShader = nullptr,
			RHIGeometryShader* inGeometryShader = nullptr
		)
			:mVertexDeclaration(inVertexDeclaration),
			mVertexShader(inVertexShader),
			mPixelShader(inPixelShader),
			mHullShader(inHullShader),
			mDomainShader(inDomainShader),
			mGeometryShader(inGeometryShader)
		{}

		friend bool operator == (const BoundShaderStateKey& A, const BoundShaderStateKey& B)
		{
			return A.mVertexDeclaration == B.mVertexDeclaration &&
				A.mVertexShader == B.mVertexShader&&
				A.mPixelShader == B.mPixelShader&&
				A.mHullShader == B.mHullShader&&
				A.mDomainShader == B.mDomainShader&&
				A.mGeometryShader == B.mGeometryShader;
		}

		friend uint32 getTypeHash(const BoundShaderStateKey & key)
		{
			return getTypeHash(key.mVertexDeclaration.getReference()) ^
				getTypeHash(key.mVertexShader) ^
				getTypeHash(key.mPixelShader) ^
				getTypeHash(key.mHullShader) ^
				getTypeHash(key.mDomainShader) ^
				getTypeHash(key.mGeometryShader);
		}

		FORCEINLINE RHIVertexShader* getVertexShader() const
		{
			return mVertexShader;
		}

		FORCEINLINE RHIPixelShader* getPixelShader() const
		{
			return mPixelShader;
		}

		FORCEINLINE RHIHullShader* getHullShader() const
		{
			return mHullShader;
		}

		FORCEINLINE RHIDomainShader* getDomainShader() const
		{
			return mDomainShader;
		}

		FORCEINLINE RHIGeometryShader* getGeometryShader() const
		{
			return mGeometryShader;
		}

		friend bool operator != (const BoundShaderStateKey & x, const BoundShaderStateKey &y)
		{
			return !(x == y);
		}



	private:
		VertexDeclarationRHIRef mVertexDeclaration;
		VertexShaderRHIRef mVertexShader;
		PixelShaderRHIRef mPixelShader;
		HullShaderRHIRef mHullShader;
		DomainShaderRHIRef mDomainShader;
		GeometryShaderRHIRef mGeometryShader;
	};
}

namespace Air
{




	class RHI_API CachedBoundShaderStateLink
	{
	public:
		RHIBoundShaderState* mBoundShaderState;
		CachedBoundShaderStateLink(
			RHIVertexDeclaration* vertexDeclaration,
			RHIVertexShader* vertexShader,
			RHIPixelShader* pixelShader,
			RHIBoundShaderState* inBoundShaderState,
			bool bAddToSingleThreadedCache = true);

		CachedBoundShaderStateLink(
			RHIVertexDeclaration* vertexDeclaration,
			RHIVertexShader* vertexShader,
			RHIPixelShader* pixelShader,
			RHIHullShader* hullShader,
			RHIDomainShader* domainShader,
			RHIGeometryShader* geometryShader,
			RHIBoundShaderState* inBoundShaderState,
			bool bAddToSingleThreadedCached = true
		);

		~CachedBoundShaderStateLink();

		FORCEINLINE RHIVertexShader* getVertexShader() const
		{
			return mKey.getVertexShader();
		}

		FORCEINLINE RHIPixelShader* getPixelShader() const
		{
			return mKey.getPixelShader();
		}

		FORCEINLINE RHIHullShader* getHullShader() const
		{
			return mKey.getHullShader();
		}

		FORCEINLINE RHIDomainShader* getDomainShader() const
		{
			return mKey.getDomainShader();
		}

		FORCEINLINE RHIGeometryShader* getGeometryShader() const
		{
			return mKey.getGeometryShader();
		}
	protected:
		BoundShaderStateKey mKey;
		bool bAddedToSingleThreadedCache;
	};

	class RHI_API CachedBoundShaderStateLink_ThreadSafe : public CachedBoundShaderStateLink
	{
	public:
		CachedBoundShaderStateLink_ThreadSafe(
			RHIVertexDeclaration* vertexDeclaration,
			RHIVertexShader* vertexShader,
			RHIPixelShader* pixelShader,
			RHIBoundShaderState* inBoundShaderState
		)
			:CachedBoundShaderStateLink(vertexDeclaration, vertexShader, pixelShader, inBoundShaderState, false)
		{

		}
		CachedBoundShaderStateLink_ThreadSafe(RHIVertexDeclaration* vertexDeclaration,
			RHIVertexShader* vertexShader,
			RHIPixelShader* pixelShader,
			RHIHullShader* hullShader,
			RHIDomainShader* domainShader,
			RHIGeometryShader* geometryShader,
			RHIBoundShaderState* inBoundShaderState)
			:
			CachedBoundShaderStateLink(vertexDeclaration, vertexShader, pixelShader, hullShader, domainShader, geometryShader, inBoundShaderState, false)
		{}
		void addToCache();
		void removeFromCache();
	};


	extern RHI_API 	CachedBoundShaderStateLink* getCachedBoundShaderState(
		RHIVertexDeclaration* vertexDeclaration,
		RHIVertexShader* vertexShader,
		RHIPixelShader* pixelShader,
		RHIHullShader* hullShader,
		RHIDomainShader* domainShader,
		RHIGeometryShader* geometryShader
	);

	extern RHI_API BoundShaderStateRHIRef getCachedBoundShaderState_ThreadSafe(RHIVertexDeclaration* vertexDeclaration,
		RHIVertexShader* vertexShader,
		RHIPixelShader* pixelShader,
		RHIHullShader* hullShader,
		RHIDomainShader* domainShader,
		RHIGeometryShader* geometryShader);

	extern RHI_API void emptyCachedBoundShaderStates();
}

