#pragma once
#include "RHIConfig.h"
#include "RHIResource.h"
namespace Air
{
	class BoundShaderStateKey
	{
	public:
		BoundShaderStateKey(
			VertexDeclarationRHIParamRef inVertexDeclaration,
			VertexShaderRHIParamRef inVertexShader,
			PixelShaderRHIParamRef inPixelShader,
			HullShaderRHIParamRef inHullShader = nullptr,
			DomainShaderRHIParamRef inDomainShader = nullptr,
			GeometryShaderRHIParamRef inGeometryShader = nullptr
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

		FORCEINLINE VertexShaderRHIParamRef getVertexShader() const
		{
			return mVertexShader;
		}

		FORCEINLINE PixelShaderRHIParamRef getPixelShader() const
		{
			return mPixelShader;
		}

		FORCEINLINE HullShaderRHIParamRef getHullShader() const
		{
			return mHullShader;
		}

		FORCEINLINE DomainShaderRHIParamRef getDomainShader() const
		{
			return mDomainShader;
		}

		FORCEINLINE GeometryShaderRHIParamRef getGeometryShader() const
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
		BoundShaderStateRHIParamRef mBoundShaderState;
		CachedBoundShaderStateLink(
			VertexDeclarationRHIParamRef vertexDeclaration,
			VertexShaderRHIParamRef vertexShader,
			PixelShaderRHIParamRef pixelShader,
			BoundShaderStateRHIParamRef inBoundShaderState,
			bool bAddToSingleThreadedCache = true);

		CachedBoundShaderStateLink(
			VertexDeclarationRHIParamRef vertexDeclaration,
			VertexShaderRHIParamRef vertexShader,
			PixelShaderRHIParamRef pixelShader,
			HullShaderRHIParamRef hullShader,
			DomainShaderRHIParamRef domainShader,
			GeometryShaderRHIParamRef geometryShader,
			BoundShaderStateRHIParamRef inBoundShaderState,
			bool bAddToSingleThreadedCached = true
		);

		~CachedBoundShaderStateLink();

		FORCEINLINE VertexShaderRHIParamRef getVertexShader() const
		{
			return mKey.getVertexShader();
		}

		FORCEINLINE PixelShaderRHIParamRef getPixelShader() const
		{
			return mKey.getPixelShader();
		}

		FORCEINLINE HullShaderRHIParamRef getHullShader() const
		{
			return mKey.getHullShader();
		}

		FORCEINLINE DomainShaderRHIParamRef getDomainShader() const
		{
			return mKey.getDomainShader();
		}

		FORCEINLINE GeometryShaderRHIParamRef getGeometryShader() const
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
			VertexDeclarationRHIParamRef vertexDeclaration,
			VertexShaderRHIParamRef vertexShader,
			PixelShaderRHIParamRef pixelShader,
			BoundShaderStateRHIParamRef inBoundShaderState
		)
			:CachedBoundShaderStateLink(vertexDeclaration, vertexShader, pixelShader, inBoundShaderState, false)
		{

		}
		CachedBoundShaderStateLink_ThreadSafe(VertexDeclarationRHIParamRef vertexDeclaration,
			VertexShaderRHIParamRef vertexShader,
			PixelShaderRHIParamRef pixelShader,
			HullShaderRHIParamRef hullShader,
			DomainShaderRHIParamRef domainShader,
			GeometryShaderRHIParamRef geometryShader,
			BoundShaderStateRHIParamRef inBoundShaderState)
			:
			CachedBoundShaderStateLink(vertexDeclaration, vertexShader, pixelShader, hullShader, domainShader, geometryShader, inBoundShaderState, false)
		{}
		void addToCache();
		void removeFromCache();
	};


	extern RHI_API 	CachedBoundShaderStateLink* getCachedBoundShaderState(
		VertexDeclarationRHIParamRef vertexDeclaration,
		VertexShaderRHIParamRef vertexShader,
		PixelShaderRHIParamRef pixelShader,
		HullShaderRHIParamRef hullShader,
		DomainShaderRHIParamRef domainShader,
		GeometryShaderRHIParamRef geometryShader
	);

	extern RHI_API BoundShaderStateRHIRef getCachedBoundShaderState_ThreadSafe(VertexDeclarationRHIParamRef vertexDeclaration,
		VertexShaderRHIParamRef vertexShader,
		PixelShaderRHIParamRef pixelShader,
		HullShaderRHIParamRef hullShader,
		DomainShaderRHIParamRef domainShader,
		GeometryShaderRHIParamRef geometryShader);
}

