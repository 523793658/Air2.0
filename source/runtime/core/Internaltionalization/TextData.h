#pragma once
#include "CoreType.h"
#include "Internaltionalization/ITextData.h"
namespace Air
{
	template<typename THistoryType>
	class TTextData : public ITextData
	{
	public:
		TTextData()
			:mLocalizedString(),mHistory()
		{}

		explicit TTextData(TextDisplayStringPtr inLocalizedString)
			:mLocalizedString(std::move(inLocalizedString))
			,mHistory()
		{}

	protected:
		TextDisplayStringPtr mLocalizedString;
		THistoryType mHistory;

	};



	template<typename THistoryType>
	class TLocalizedTextData : public TTextData<THistoryType>
	{
	public:
		TLocalizedTextData()
		{}
		explicit TLocalizedTextData(TextDisplayStringPtr inLocalizedString)
			:TTextData<THistoryType>(inLocalizedString)
		{}
	};
}