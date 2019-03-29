#pragma once
#include "GenericPlatform/GenericApplication.h"
#include "Windows/MinimalWindowsApi.h"
#include "Containers/Array.h"
#include "Windows/WindowsHWrapper.h"
namespace Air
{
	class WindowsWindow;
	class IWindowsMessageHandler
	{
	public:
		virtual bool processMessage(HWND hwnd, uint32, WPARAM wParam, LPARAM lParam, int32& outResult) = 0;
	};

	struct DragDropOLEData
	{
		enum EWindowsOLEDataType
		{
			None = 0,
			Text = 1 << 0,
			Files = 1 << 1,
		};
		DragDropOLEData()
			:mType(None)
		{}
		bool isUnicode{ true };
		wstring mOperationText;
		string mOperationTextAsic;
		TArray<wstring> mOperationFilenames;

		uint8 mType;
	};


	struct DeferredWindowsMessage 
	{
		DeferredWindowsMessage(const std::shared_ptr<WindowsWindow>& inNativeWindow, HWND inHWnd, uint32 inMessage, WPARAM inWParam, LPARAM inLParam, int32 inX, int32 inY, uint32 inRawInputFlags = 0)
			:mNativeWindow(inNativeWindow)
			,mHWND(inHWnd)
			,mMessage(inMessage)
			,mWParam(inWParam)
			,mLParam(inLParam)
			,mX(inX)
			,mY(inY)
			,mRawInputFlags(inRawInputFlags)
		{}

		std::weak_ptr<WindowsWindow> mNativeWindow;
		HWND mHWND;
		uint32 mMessage;
		WPARAM mWParam;
		LPARAM mLParam;

		int32 mX;
		int32 mY;
		uint32 mRawInputFlags;
	};


	class CORE_API WindowsApplication
		: public GenericApplication
	{
	public:
		static WindowsApplication* createWindowsApplication(const HINSTANCE instatnceHandle, const HICON iconHandle);

		void initializeWindow(const std::shared_ptr<GenericWindow>& window, const std::shared_ptr<GenericWindowDefinition>& inDesc, const std::shared_ptr<GenericWindow>& inParent, const bool bShowImmediately);

		virtual void addMessageHandler(IWindowsMessageHandler& inMessageHandler);

		virtual std::shared_ptr<GenericWindow> makeWindow() override;

		virtual ~WindowsApplication();

		virtual ModifierKeysState getModifierKeys() const override;

		virtual void setMessageHandler(const std::shared_ptr<GenericApplicationMessageHandler>& inMessageHandler) override;

		virtual void setCapture(const std::shared_ptr<GenericWindow>& inWindow) override;

		virtual void* getCapture() const;

		virtual void setHighPrecisionMouseMode(const bool enable, const std::shared_ptr<GenericWindow>& inWindow);

		virtual void pollGameDeviceState(const float timeDelta) override;
	private:
		WindowsApplication(const HINSTANCE instanceHandle, const HICON iconHandle);
	protected:
		static LRESULT CALLBACK appWndProc(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam);

		int32 processMessage(HWND hwnd, uint32 msg, WPARAM wParam, LPARAM lParam);


	private:
		bool registerClass(const HINSTANCE instanceHandle, const HICON iconHandle);

		void deferMessage(std::shared_ptr<WindowsWindow>& nativeWindow, HWND inHwnd, uint32 inMessage, WPARAM inWParam, LPARAM inLParam, int32 mouseX = 0, int32 mouseY = 0, uint32 rawInputFlags = 0);

		bool isInputMessage(uint32 msg);

		int32 processDeferredMessage(const DeferredWindowsMessage& deferredMessage);

		bool isKeyboardInputMessage(uint32 msg);

		bool isMouseInputMessage(uint32 msg);

		void updateAllModifierKeyStates();

		virtual bool isCursorDirectlyOverSlateWindow() const override;

	public:
		HRESULT onOLEDragEnter(const HWND hWnd, const DragDropOLEData& oleData, ::DWORD KeyState, POINTL cursorPositon, ::DWORD* cursorEffect);
		HRESULT onOLEDragOver(const HWND HWnd, ::DWORD KeyState, POINTL CursorPosition, ::DWORD *CursorEffect);

		/** Invoked by a window when an OLE Drag and Drop exits the window. */
		HRESULT onOLEDragOut(const HWND HWnd);

		/** Invoked by a window when an OLE Drag and Drop is dropped onto the window. */
		HRESULT onOLEDrop(const HWND HWnd, const DragDropOLEData& OLEData, ::DWORD KeyState, POINTL CursorPosition, ::DWORD *CursorEffect);
	private:
		TArray<std::shared_ptr<class WindowsWindow>> mWindows;

		HINSTANCE mInstanceHandle;

		TArray<IWindowsMessageHandler*> mMessageHandlers;
		bool bInModalSizeLoop{ false };

		TArray<DeferredWindowsMessage> mDeferredMessages;

		int32 bAllowToDeferMessageProcessing{ 1 };


		static const int2 MinimizedWindowPosition;

		struct EModifierKey 
		{
			enum Type
			{
				LeftShift,
				RightShift,
				LeftControl,
				RightControl,
				LeftAlt,
				RightAlt,
				CapsLock,
				Count,
			};
		};

		bool mModifierKeyState[EModifierKey::Count];
		bool bUsingHighPrecisionMouseInput{ false };
		bool bForceActivateByMouse{ false };

	};
}