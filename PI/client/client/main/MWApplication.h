#ifndef _MWApplication_H_
#define _MWApplication_H_

class MWApplication
{
private:

	// ��һ��ָ�룬���ʻ᷽��Щ
	PiGameServiceManager *mpServiceManager;

public:

	// ���캯��
	MWApplication();

	// ��������
	~MWApplication();

	// Singleton
	static MWApplication * GetInstance();

	// ��ʼ��
	bool Initialize();

	// ����
	void Finalize();

	// ����
	void Update(const float vfElapsedTime);

	// ǿ���˳�����
	void QuitApplication(const int vnExitCode);
};

#endif