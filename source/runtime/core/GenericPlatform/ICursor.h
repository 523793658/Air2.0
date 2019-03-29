#pragma once
#include "CoreType.h"
#include "Math/Vector.h"
struct tagRECT;
typedef struct tagRECT RECT;

namespace Air
{
	

	namespace EMouseCursor
	{
		enum Type
		{
			None,
			Default,
			TextEditBeam,
			ResizeLeftRight,
			ResizeUpDown,
			ResizeSouthEast,
			ResizeSouthWest,
			CardinalCross,
			Crosshairs,
			Hand,
			GrabHand,
			GrabHandClosed,
			SlashedCircle,
			EyeDropper,
			Custom,
			TotalCursorCount
		};
	}


	class ICursor
	{
	public:
		virtual float2 getPosition() const = 0;

		virtual void setPosition(const int32 x, const int32 y) = 0;

		virtual void setType(const EMouseCursor::Type inNewCursor) = 0;

		virtual EMouseCursor::Type getType() const = 0;

		virtual void getSize(int32& width, int32& height) const = 0;

		virtual void show(bool bShow) = 0;

		virtual void lock(const RECT* const bounds) = 0;

		virtual void setCustomShape(void* cursorHandle) = 0;
	};
}