#include "SlateApplicationBase.h"

namespace Air
{
	std::shared_ptr<SlateApplicationBase> SlateApplicationBase::mCurrentBaseApplication;
	std::shared_ptr<GenericApplication> SlateApplicationBase::mPlatformApplication;
	const uint32 SlateApplicationBase::CursorPointerIndex = EKeys::NUM_TOUCH_KEYS - 1;

	const uint32 SlateApplicationBase::CursorUseIndex = 0;
}