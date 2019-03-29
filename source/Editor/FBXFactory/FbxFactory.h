#pragma once
#include "CoreMinimal.h"
#include "Classes/Factories/Factory.h"
#include "config.h"


namespace Air
{
	


	class Fbx_API FbxFactory : public Factory
	{
	public:
		virtual void getExtensions(TCHAR**& extensions, uint32& num) override;

		virtual Object* createFromFile(Object* inObject, wstring filename, wstring name, EObjectFlags flags) override;

	protected:

	

	public:



	};
}

