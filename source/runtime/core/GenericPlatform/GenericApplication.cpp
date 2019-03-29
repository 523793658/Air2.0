#include "GenericPlatform/GenericApplication.h"
namespace Air
{
	GenericApplication::GenericApplication(const std::shared_ptr<ICursor> & inCursor)
		: mCursor(inCursor)
		, mMessageHandler(MakeSharedPtr<GenericApplicationMessageHandler>())
	{

	}

	void GenericApplication::initializeWindow(const std::shared_ptr<GenericWindow>& window, const std::shared_ptr<GenericWindowDefinition>& inDesc, const std::shared_ptr<GenericWindow>& inParent, const bool bShowImmediately)
	{

	}


}