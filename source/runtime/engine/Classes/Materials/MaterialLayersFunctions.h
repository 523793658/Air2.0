#pragma once
#include "CoreMinimal.h"
#include "EngineMininal.h"
#include "Containers/EnumAsByte.h"
namespace Air
{
	enum EMaterialParameterAssociation
	{
		LayerParameter,
		BlendParameter,
		GlobalParameter,
	};


	struct ENGINE_API MaterialParameterInfo
	{
		wstring mName;
		TEnumAsByte<EMaterialParameterAssociation> mAssociation;

		int32 mIndex;

#if WITH_EDITORONLY_DATA
		
#endif

		MaterialParameterInfo(const TCHAR* inName, EMaterialParameterAssociation inAssociation = EMaterialParameterAssociation::GlobalParameter, int32 inIndex = INDEX_NONE)
			:mName(inName)
			,mAssociation(inAssociation)
			,mIndex(inIndex)
		{}

		MaterialParameterInfo(wstring inName = TEXT(""), EMaterialParameterAssociation inAssociation = EMaterialParameterAssociation::GlobalParameter, int32 inIndex = INDEX_NONE)
			:mName(inName)
			,mAssociation(inAssociation)
			,mIndex(inIndex)
		{

		}

		wstring toString() const
		{
			return mName + boost::lexical_cast<wstring>(mAssociation) + boost::lexical_cast<wstring>(mIndex);
		}

		friend Archive& operator << (Archive& ar, MaterialParameterInfo& ref)
		{
			ar << ref.mName << ref.mAssociation << ref.mIndex;
			return ar;
		}

		FORCEINLINE bool operator == (const MaterialParameterInfo& other) const
		{
			return mName == other.mName && mAssociation == other.mAssociation && mIndex == other.mIndex;
		}

		FORCEINLINE bool operator != (const MaterialParameterInfo& other) const
		{
			return mName != other.mName || mAssociation != other.mAssociation || mIndex != other.mIndex;
		}
	};
}