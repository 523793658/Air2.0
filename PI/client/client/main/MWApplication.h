#ifndef _MWApplication_H_
#define _MWApplication_H_

class MWApplication
{
private:

	// 存一个指针，访问会方便些
	PiGameServiceManager *mpServiceManager;

public:

	// 构造函数
	MWApplication();

	// 析构函数
	~MWApplication();

	// Singleton
	static MWApplication * GetInstance();

	// 初始化
	bool Initialize();

	// 结束
	void Finalize();

	// 更新
	void Update(const float vfElapsedTime);

	// 强制退出程序
	void QuitApplication(const int vnExitCode);
};

#endif