#pragma once
#include "CoreMinimal.h"
#include "ShaderCore.h"
#include "Misc/App.h"
#include "ShaderParameterMap.h"
#include "RHIUtilities.h"
namespace Air
{
	enum EShaderParameterFlags
	{
		SPF_Optional,
		SPF_Mandatory
	};

	class ShaderParameter
	{
	public:
		ShaderParameter()
			:mBufferIndex(0)
			,mBaseIndex(0)
			,mNumBytes(0)
		{} 
		RENDER_CORE_API void bind(const ShaderParameterMap& parameterMap, const TCHAR* parameterName, EShaderParameterFlags flgas = SPF_Optional);

		friend RENDER_CORE_API Archive& operator << (Archive& ar, ShaderParameter & p);

		bool isBound() const { return mNumBytes > 0; }

		inline bool isInitialized() const 
		{
			return true;
		}
		uint32 getBufferIndex() const { return mBufferIndex; }
		uint32 getBaseIndex() const { return mBaseIndex; }
		uint32 getNumBytes() const { return mNumBytes; }


	private:
		uint16 mBufferIndex;
		uint16 mBaseIndex;
		uint16 mNumBytes;
	};

	

	class ShaderConstantBufferParameter
	{
	public:
		ShaderConstantBufferParameter()
			:mSetParametersId(0)
			, mBaseIndex(0)
			, bIsBound(false)
		{
		}

		inline bool isInitialized() const
		{
			return true;
		}

		uint32 getBaseIndex() const { return mBaseIndex; }

		RENDER_CORE_API void bind(const ShaderParameterMap& parameterMap, const TCHAR* parameterName, EShaderParameterFlags flags = SPF_Optional);

		friend Archive& operator << (Archive& ar, ShaderConstantBufferParameter& p)
		{
			p.serialize(ar);
			return ar;
		}

		void serialize(Archive& ar)
		{
			ar << mBaseIndex << bIsBound;
		}

		bool isBound() const { return bIsBound; }

		static RENDER_CORE_API void modifyCompilationEnvironment(const TCHAR* parameterName, const ShaderParametersMetadata& inStruct, EShaderPlatform platform, ShaderCompilerEnvironment& outEnvironemtn);

		mutable uint32 mSetParametersId;
	private:
		uint16 mBaseIndex;
		bool bIsBound;
	};

	template<typename TBufferStruct>
	class TShaderConstantBufferParameter : public ShaderConstantBufferParameter
	{
	public:
		static void modifyCompilationEnvironment()
		{

		}
		inline void setInitialized()
		{

		}

		
	};


	class ShaderResourceParameter
	{
	public:
		ShaderResourceParameter()
			:mBaseIndex(0),
			mNumResource(0)
		{}
		RENDER_CORE_API void bind(const ShaderParameterMap& parameterMap, const TCHAR* parameterName, EShaderParameterFlags flags = SPF_Optional);

		friend RENDER_CORE_API Archive& operator <<(Archive& ar, ShaderResourceParameter& p);

		bool isBound() const { return mNumResource > 0; }

		inline bool isInitialized() const
		{
			return true;
		}

		uint32 getBaseIndex() const { return mBaseIndex; }
		uint32 getNumResources() const { return mNumResource; }


	private:
		uint16 mBaseIndex;
		uint16 mNumResource;
	};
	class RWShaderParameter
	{
	public:
		void bind(const ShaderParameterMap& parameterMap, const TCHAR* baseName)
		{
			mSRVParameter.bind(parameterMap, baseName);
			wstring uavName = wstring(TEXT("RW")) + baseName;
			mUAVParameter.bind(parameterMap, uavName.c_str());

			BOOST_ASSERT(!(mSRVParameter.getNumResources() && mUAVParameter.getNumResources()));
		}

		bool isBound() const
		{
			return mSRVParameter.isBound() || mUAVParameter.isBound();
		}

		bool isUAVBound() const
		{
			return mUAVParameter.isBound();
		}

		uint32 getUAVIndex() const
		{
			return mUAVParameter.getBaseIndex();
		}

		friend Archive& operator <<(Archive& ar, RWShaderParameter& parameter)
		{
			return ar << parameter.mSRVParameter << parameter.mUAVParameter;
		}

		template<typename TShaderRHIRef, typename TRHICmdList>
		inline void setBuffer(TRHICmdList& RHICmdList, TShaderRHIRef shader, const RWBuffer* rwBuffer) const;

		template<typename TShaderRHIRef, typename TRHICmdList>
		inline void setBuffer(TRHICmdList& RHICmdList, TShaderRHIRef shader, const RWBufferStructured& rwBuffer) const;

		template<typename TShaderRHIRef, typename TRHICmdList>
		inline void setTexture(TRHICmdList& RHICmdList, TShaderRHIRef shader, RHITexture* texture, RHIUnorderedAccessView* uav) const;

		template<typename TRHICmdList>
		inline void unsetUAV(TRHICmdList& RHICmdList, RHIComputeShader* computeShader) const;


	private:
		ShaderResourceParameter mSRVParameter;
		ShaderResourceParameter mUAVParameter;
	};
	extern RENDER_CORE_API wstring createConstantBufferShaderDeclaration(const TCHAR* name, const ShaderParametersMetadata& ShaderParametersMetadata, EShaderPlatform platform);
}