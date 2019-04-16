#pragma once
#include "CoreType.h"
namespace Air
{
	enum EDayOfWeek
	{
		Monday = 0,
		Tuesday,
		Wednesday,
		Thursday,
		Friday,
		Saturday,
		Sunday
	};

	enum EMonthOfYear
	{
		January = 1,
		February,
		March,
		April,
		May,
		June,
		July,
		August,
		September,
		October,
		November,
		December
	};

	class DateTime
	{
	public:
	protected:
		static const int32 mDaysPerMonth;
		static const int32 mDaysToMonth;

	private:
		int64 mTicks;
	};
}