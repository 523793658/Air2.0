#include "StaticParameterSet.h"
#include "Misc/SecureHash.h"
#include "boost/lexical_cast.hpp"
namespace Air
{
	void StaticParameterSet::updateHash(SHA1& hashStage)const
	{
		for (int32 paramIndex = 0; paramIndex < mStaticSwitchParameters.size(); paramIndex++)
		{
			const StaticSwitchParameter& switchParameter = mStaticSwitchParameters[paramIndex];
			const wstring parameterName = switchParameter.mParameterName;
			hashStage.update((const uint8*)parameterName.c_str(), parameterName.length() * sizeof(TCHAR));
			hashStage.update((const uint8*)&switchParameter.mExpressionGUID, sizeof(switchParameter.mExpressionGUID));
			hashStage.update((const uint8*)&switchParameter.mValue, sizeof(switchParameter.mValue));
		}

		for (int32 paramIndex = 0; paramIndex < mStaticComponentMaskParameters.size(); paramIndex++)
		{
			const StaticComponentMaskParameter& componenentMaskParameter = mStaticComponentMaskParameters[paramIndex];
			const wstring parameterName = componenentMaskParameter.mParameterName;
			hashStage.update((const uint8*)parameterName.c_str(), parameterName.length() * sizeof(TCHAR));
			hashStage.update((const uint8*)&componenentMaskParameter.mExpressionGUID, sizeof(componenentMaskParameter.mExpressionGUID));
			hashStage.update((const uint8*)&componenentMaskParameter.R, sizeof(componenentMaskParameter.R));
			hashStage.update((const uint8*)&componenentMaskParameter.G, sizeof(componenentMaskParameter.G));
			hashStage.update((const uint8*)&componenentMaskParameter.B, sizeof(componenentMaskParameter.B));
			hashStage.update((const uint8*)&componenentMaskParameter.A, sizeof(componenentMaskParameter.A));
		}
	}

	void StaticParameterSet::appendKeyString(wstring & keyString) const
	{
		for (int32 paramIndex = 0; paramIndex < mStaticSwitchParameters.size(); paramIndex++)
		{
			const StaticSwitchParameter& switchParameter = mStaticSwitchParameters[paramIndex];
			keyString += switchParameter.mParameterName + switchParameter.mExpressionGUID.toString() + boost::lexical_cast<wstring>(switchParameter.mValue);
		}

		for (int32 paramIndex = 0; paramIndex < mStaticComponentMaskParameters.size(); paramIndex++)
		{
			const StaticComponentMaskParameter& componentMaskParameter = mStaticComponentMaskParameters[paramIndex];
			keyString += componentMaskParameter.mParameterName + componentMaskParameter.mExpressionGUID.toString() + boost::lexical_cast<wstring>(componentMaskParameter.R) + boost::lexical_cast<wstring>(componentMaskParameter.G) + boost::lexical_cast<wstring>(componentMaskParameter.B) + boost::lexical_cast<wstring>(componentMaskParameter.A);
		}
	}

	bool StaticParameterSet::operator == (const StaticParameterSet& referenceSet) const
	{
		if (mStaticSwitchParameters.size() == referenceSet.mStaticSwitchParameters.size() && mStaticComponentMaskParameters.size() == referenceSet.mStaticComponentMaskParameters.size())
		{
			for (int32 switchIndex = 0; switchIndex < mStaticSwitchParameters.size()
				; switchIndex++)
			{
				if (mStaticSwitchParameters[switchIndex].mParameterName != referenceSet.mStaticSwitchParameters[switchIndex].mParameterName || mStaticSwitchParameters[switchIndex].mExpressionGUID != referenceSet.mStaticSwitchParameters[switchIndex].mExpressionGUID ||
					mStaticSwitchParameters[switchIndex].mValue != referenceSet.mStaticSwitchParameters[switchIndex].mValue)
				{
					return false;
				}
			}

			for (int32 componentMaskIndex = 0; componentMaskIndex < mStaticComponentMaskParameters.size(); componentMaskIndex++)
			{
				if (mStaticComponentMaskParameters[componentMaskIndex].mParameterName != referenceSet.mStaticComponentMaskParameters[componentMaskIndex].mParameterName
					|| mStaticComponentMaskParameters[componentMaskIndex].mExpressionGUID != referenceSet.mStaticComponentMaskParameters[componentMaskIndex].mExpressionGUID ||
					mStaticComponentMaskParameters[componentMaskIndex].R != referenceSet.mStaticComponentMaskParameters[componentMaskIndex].R ||
					mStaticComponentMaskParameters[componentMaskIndex].G != referenceSet.mStaticComponentMaskParameters[componentMaskIndex].G ||
					mStaticComponentMaskParameters[componentMaskIndex].B != referenceSet.mStaticComponentMaskParameters[componentMaskIndex].B ||
					mStaticComponentMaskParameters[componentMaskIndex].A != referenceSet.mStaticComponentMaskParameters[componentMaskIndex].A)
				{
					return false;
				}
			}
			return true;
		}
		return false;
	}
}