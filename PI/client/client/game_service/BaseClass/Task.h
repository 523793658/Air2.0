#ifndef _TASK_H_
#define _TASK_H_

// 异步任务基类，所有需要异步执行的任务继承Task，实现Run、Callback
namespace pi
{

class Task
{
public:
	Task() {}
	virtual ~Task() {}
	virtual bool Run() = 0;
	virtual void Callback() = 0;
};

}
#endif 