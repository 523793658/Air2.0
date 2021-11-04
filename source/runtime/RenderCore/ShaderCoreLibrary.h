#pragma once
#include "CoreType.h"
#include "RenderCore.h"
#include "Containers/StringFwd.h"
#include "HAL/StringView.h"
#include "Misc/SecureHash.h"

namespace Air
{

	class ShaderPipeline;
	struct RENDER_CORE_API FCompactFullName
	{
		TArray<wstring, TInlineAllocator<16>> ObjectClassAndPath;

		bool operator==(const FCompactFullName& Other) const
		{
			return ObjectClassAndPath == Other.ObjectClassAndPath;
		}

		wstring toString() const;
		void appendString(StringBuilderBase& Out) const;
		void appendString(WideStringBuilderBase& Out) const;
		void parseFromString(const string_view& Src);
		friend RENDER_CORE_API uint32 getTypeHash(const FCompactFullName& A);
	};

	struct RENDER_CORE_API StableShaderKeyAndValue
	{
		FCompactFullName ClassNameAndObjectPath;
		wstring ShaderType;
		wstring ShaderClass;
		wstring MaterialDomain;
		wstring FeatureLevel;
		wstring QualityLevel;
		wstring TargetFrequency;
		wstring TargetPlatform;
		wstring VFType;
		wstring PermutationId;
		SHAHash PipelineHash;

		uint32 KeyHash;

		SHAHash OutputHash;

		StableShaderKeyAndValue()
			: KeyHash(0)
		{
		}

		void computeKeyHash();
		void parseFromString(const wstring_view& Src);
		void parseFromStringCached(const wstring_view& Src, class TMap<uint32, wstring>& NameCache);
		wstring ToString() const;
		void ToString(wstring& OutResult) const;
		void AppendString(WideStringBuilderBase& Out) const;
		static wstring HeaderLine();

		/** Computes pipeline hash from the passed pipeline. Pass nullptr to clear */
		void SetPipelineHash(const ShaderPipeline* Pipeline);

		friend bool operator ==(const StableShaderKeyAndValue& A, const StableShaderKeyAndValue& B)
		{
			return
				A.ClassNameAndObjectPath == B.ClassNameAndObjectPath &&
				A.ShaderType == B.ShaderType &&
				A.ShaderClass == B.ShaderClass &&
				A.MaterialDomain == B.MaterialDomain &&
				A.FeatureLevel == B.FeatureLevel &&
				A.QualityLevel == B.QualityLevel &&
				A.TargetFrequency == B.TargetFrequency &&
				A.TargetPlatform == B.TargetPlatform &&
				A.VFType == B.VFType &&
				A.PermutationId == B.PermutationId &&
				A.PipelineHash == B.PipelineHash;
		}

		friend uint32 GetTypeHash(const StableShaderKeyAndValue& Key)
		{
			return Key.KeyHash;
		}

	};

}