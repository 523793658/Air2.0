#include "Events.h"
namespace Air
{
	const TouchKeySet TouchKeySet::StandardSet(EKeys::LeftMouseButton);

	const TouchKeySet TouchKeySet::EmptySet(EKeys::Invalid);

	bool InputEvent::isPointerEvent() const
	{
		return false;
	}

	bool PointerEvent::isPointerEvent() const
	{
		return true;
	}
}