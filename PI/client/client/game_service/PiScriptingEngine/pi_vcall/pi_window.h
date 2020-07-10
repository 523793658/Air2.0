#ifndef _PI_WINDOW_H_
#define _PI_WINDOW_H_


#include "v8.h"
#include "nan/nan.h"

class WindowBinding
{
public:
	//添加window是事件
	static NAN_METHOD(AddEventListener);
	//设置窗口大小
	static NAN_METHOD(setWindowSize);
	//设置窗口标题
	static NAN_METHOD(setWindowTitle);
	//执行窗口闪烁
	static NAN_METHOD(flashWindow);
	//获取窗口大小return {w:1,height:1}
	static int getWindowSize();
	//改变全屏状态
	static NAN_METHOD(toggleFullScreen);
	//执行最大化
	static NAN_METHOD(setWindowsShow);
	//显示隐藏窗口
	static NAN_METHOD(showMainWindow);
	//启用、禁用输入法
	static NAN_METHOD(immEnable);
	//销毁窗口
	static NAN_METHOD(destroyWindow);
	//通过Key值获取命令行参数
	static NAN_METHOD(getCommandByKey);
	//打开默认浏览器
	static NAN_METHOD(openUrl);
	//获取启动程序的微秒数
	static NAN_METHOD(getMachineTime);
	
};


#endif