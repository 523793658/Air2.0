#ifndef _TASK_H_
#define _TASK_H_

// �첽������࣬������Ҫ�첽ִ�е�����̳�Task��ʵ��Run��Callback
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