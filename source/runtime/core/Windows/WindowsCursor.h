#pragma once
#include "GenericPlatform/ICursor.h"
#include "windows/MinimalWindowsApi.h"
namespace Air
{
	class WindowsCursor : public ICursor
	{
	public:
		WindowsCursor();
		virtual ~WindowsCursor();

		virtual float2 getPosition() const override;

		virtual void setPosition(const int32 x, const int32 y) override;

		virtual void setType(const EMouseCursor::Type inNewCursor) override;

		virtual EMouseCursor::Type getType() const override
		{
			return mCurrentType;
		}

		virtual void getSize(int32& width, int32& height) const override;

		virtual void show(bool bShow) override;

		virtual void lock(const RECT* const bounds) override;

		virtual void setCustomShape(void* cursorHandle) override;

	private:
		EMouseCursor::Type mCurrentType;
		Windows::HCURSOR mCursorHandles[EMouseCursor::TotalCursorCount];
	};
}