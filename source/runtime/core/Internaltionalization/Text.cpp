#include "Internaltionalization/Text.h"
#include "Internaltionalization/TextData.h"
#include "Internaltionalization/TextHistory.h"

namespace Air
{
	Text::Text(EInitToEmptyString)
		:mTextData(new TLocalizedTextData<TextHistory_Base>(MakeSharedPtr<wstring>()))
		,mFlags(0)
	{

	}

	Text::Text()
		:mTextData(getEmpty().mTextData)
		,mFlags(0)
	{}
}