#pragma once
#include "EngineMininal.h"
#include "Misc/Guid.h"
#include "Misc/SecureHash.h"
namespace Air
{
	class StaticSwitchParameter
	{
	public:
		wstring mParameterName;
		bool mValue;
		bool bOverride;
		Guid mExpressionGUID;
		StaticSwitchParameter()
			:mParameterName(TEXT("None")),
			mValue(false),
			bOverride(false),
			mExpressionGUID(0, 0, 0, 0)
		{}

		StaticSwitchParameter(wstring inName, bool inValue, bool inOverride, Guid inGuid)
			:mParameterName(inName),
			mValue(inValue),
			bOverride(inOverride),
			mExpressionGUID(inGuid)
		{

		}
		friend Archive& operator << (Archive& ar, StaticSwitchParameter& p)
		{
			ar << p.mParameterName << p.mValue << p.bOverride << p.mExpressionGUID;
			return ar;
		}
	};

	class StaticComponentMaskParameter
	{
	public:
		wstring mParameterName;
		bool R, G, B, A;
		bool bOverride;
		Guid mExpressionGUID;

		StaticComponentMaskParameter():mParameterName(TEXT("None")),
			R(false),
			G(false),
			B(false),
			A(false),
			bOverride(false),
			mExpressionGUID(0, 0, 0, 0)
		{}


		StaticComponentMaskParameter(wstring inName, bool inR, bool inG, bool inB, bool inA, bool bInOverride, Guid inGuid)
			:mParameterName(inName)
			,R(inR)
			,G(inG)
			,B(inB)
			,A(inA)
			,bOverride(bInOverride)
			,mExpressionGUID(inGuid)
		{}

		friend Archive& operator << (Archive& ar, StaticComponentMaskParameter & p)
		{
			ar << p.mParameterName << p.R << p.G << p.B << p.A << p.bOverride << p.mExpressionGUID;
			return ar;
		}
	};

	class StaticParameterSet
	{
	public:
		TArray<StaticSwitchParameter> mStaticSwitchParameters;

		TArray<StaticComponentMaskParameter> mStaticComponentMaskParameters;

		StaticParameterSet() {}

		bool isEmpty() const
		{
			return mStaticSwitchParameters.size() == 0 && mStaticComponentMaskParameters.size() == 0;
		}

		bool operator == (const StaticParameterSet& referenceSet)const;

		bool operator != (const StaticParameterSet& referenceSet) const
		{
			return !(*this == referenceSet);
		}

		void appendKeyString(wstring & keyString) const;

		void updateHash(SHA1& hashStage) const;

		void serialize(Archive& ar)
		{
			ar << mStaticSwitchParameters << mStaticComponentMaskParameters;
		}
	};
}