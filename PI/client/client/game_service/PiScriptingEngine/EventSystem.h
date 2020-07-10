#ifndef _EVENT_SYSTEM_H_
#define _EVENT_SYSTEM_H_

#include <string>
#include <unordered_map>
#include "game_service/PiScriptingEngine/tasks/V8Task.h"


class V8Service;
class EventSystem
{
public:
	using EventMapType = std::unordered_map<std::string, CallBackType>;

	enum class MouseType
	{
		LBUTTON = 0,
		MBUTTON,
		RBUTTON
	};

	static EventSystem& GetInstance()
	{
		static EventSystem event;
		return event;
	}

	EventSystem();
	~EventSystem();

	void SetEventCallback(const std::string& eventName, const CallBackType& cb);
	void Finalize();

	bool OnButtonDown(int leftFlag, int rightFlag);
	bool OnLButtonUp(int x, int y);
	bool OnLButtonDown(int x, int y);
	bool OnMButtonUp(int x, int y);
	bool OnMButtonDown(int x, int y);
	bool OnRButtonUp(int x, int y);
	bool OnRButtonDown(int x, int y);
	bool OnLDoubleClicked(int x, int y);
	bool OnMouseMove(int x, int y);
	bool OnMouseWheel(int x, int y, int delta);
	bool OnClose();
	bool OnMinSize();
	bool OnMaxSize(int x, int y);
	bool OnRestored(int x, int y);
	bool OnActivaterApp(int arg);
	bool OnKeyDown(int vkCode);
	bool OnKeyUp(int vkCode);

private:
	bool OnMouseEvent(const std::string& eventName, MouseType type, int x, int y);
	bool OnEventHelper(const std::string& eventName, int x, int y);
	bool OnEventHelper(const std::string& eventName, int arg);
	bool IsKeyDown(int vkKey);
	bool IsKeyUp(int vkKey);

private:
	V8Service* v8Service_;
	EventMapType map_;
};

#endif