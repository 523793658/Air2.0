#ifndef _PiScriptingEngineService_H_
#define _PiScriptingEngineService_H_

#include "game_service/BaseClass/PiGameService.h"

class PiBindings;
class V8Service;
class PiScriptingEngineService : public PiGameService
{
private:

	PiBindings *bindings;
	V8Service *mpV8Service;

public:

	// 构造函数
	PiScriptingEngineService();

	// 析构函数
	virtual ~PiScriptingEngineService();

	// 预初始化，用于同步的、无相互依赖的初始化，以及注册依赖关系
	virtual SyncResult PreInitialize();

	// 初始化，如果返回Pending，则会下一帧继续调用，Complete表示结束
	virtual AsyncResult Initialize();

	// 结束并释放资源，如果返回Pending，则会下一帧继续调用，Complete表示结束
	virtual AsyncResult Finalize();

	// 帧更新，如果返回Pending，则会继续收到更新回调
	// 如果返回Complete，则以后不再更新这个Service，如果返回Failure，则结束程序
	virtual AsyncResult Update(const float vfElaspedTime);

	// 获取显示名称，可以用来外部获取标示、调试、Log用
	virtual std::wstring GetDisplayName();

	// 执行js文件, 暴露给底层使用来加载主js
	void executeJS(const char* path, const char* data);

	// 执行主JS
	void startMainJS();
};


#endif