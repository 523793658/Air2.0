// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"
#include "SlateCoreConfig.h"
UINTERFACE()
class SLATECORE_API USlateWidgetStyleContainerInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY(USlateWidgetStyleContainerInterface, , UInterface)
};

class ISlateWidgetStyleContainerInterface
{
	GENERATED_IINTERFACE_BODY(ISlateWidgetStyleContainerInterface, SLATECORE_API, USlateWidgetStyleContainerInterface)

public:

	virtual const struct FSlateWidgetStyle* const GetStyle() const = 0;
};
