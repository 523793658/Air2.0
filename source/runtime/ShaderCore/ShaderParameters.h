#pragma once
#include "CoreMinimal.h"
#include "ShaderCore.h"
#include "Misc/App.h"
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
		SHADER_CORE_API void bind(const ShaderParameterMap& parameterMap, const TCHAR* parameterName, EShaderParameterFlags flgas = SPF_Optional);

		friend SHADER_CORE_API Archive& operator << (Archive& ar, ShaderParameter & p);

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

		SHADER_CORE_API void bind(const ShaderParameterMap& parameterMap, const TCHAR* parameterName, EShaderParameterFlags flags = SPF_Optional);

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

		static SHADER_CORE_API void modifyCompilationEnvironment(const TCHAR* parameterName, const ConstantBufferStruct& inStruct, EShaderPlatform platform, ShaderCompilerEnvironment& outEnvironemtn);

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
		SHADER_CORE_API void bind(const ShaderParameterMap& parameterMap, const TCHAR* parameterName, EShaderParameterFlags flags = SPF_Optional);

		friend SHADER_CORE_API Archive& operator <<(Archive& ar, ShaderResourceParameter& p);

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

	extern SHADER_CORE_API wstring createConstantBufferShaderDeclaration(const TCHAR* name, const ConstantBufferStruct& constantBufferStruct, EShaderPlatform platform);
}