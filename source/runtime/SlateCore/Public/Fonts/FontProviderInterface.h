// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Interface.h"
#include "SlateCoreConfig.h"
struct FCompositeFont;

//UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UFontProviderInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY(UFontProviderInterface, SLATECORE_API)
};

class IFontProviderInterface
{
	GENERATED_IINTERFACE_BODY(IFontProviderInterface, SLATECORE_API, UFontProviderInterface)

	virtual const FCompositeFont* GetCompositeFont() const = 0;
};
