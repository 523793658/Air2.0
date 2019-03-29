#pragma once
#include "SlateCore.h"
#include "SCompoundWidget.h"
#include "DeclarativeSyntaxSupport.h"
namespace Air
{
	class SUserWidget : public SCompoundWidget
	{
	public:
		struct Arguments : public TSlateBaseNamedArgs<SUserWidget>
		{
			typedef Arguments WidgetArgsType;
			FORCENOINLINE Arguments()
			SLATE_DEC
		};
	};
}