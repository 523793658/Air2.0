#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
namespace Air
{
	class IShaderFormat;

	class IShaderFormatModule : public IModuleInterface
	{
	public:
		virtual IShaderFormat * getShaderFormat() = 0;
	public:
		~IShaderFormatModule() {}
	};
}