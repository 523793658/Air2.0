#ifndef _DinoUIGameService_H_
#define _DinoUIGameService_H_
#include "game_service/BaseClass/PiGameService.h"

class DinoUIGameService : public PiGameService
{
private:

	DinoUIEngine *mpUIEngine;

	// ����һ��Windows�Ĵ��ھ��
	HWND mhWnd;

	TCHAR *mpClipboardBuffer;

public:

	bool mbInitialized;

	// ���캯��
	DinoUIGameService();

	// ��������
	virtual ~DinoUIGameService();

	// Ԥ��ʼ��������ͬ���ġ����໥�����ĳ�ʼ�����Լ�ע��������ϵ
	virtual SyncResult PreInitialize();

	// ��ʼ�����������Pending�������һ֡�������ã�Complete��ʾ����
	virtual AsyncResult Initialize();

	// �������ͷ���Դ���������Pending�������һ֡�������ã�Complete��ʾ����
	virtual AsyncResult Finalize();

	// ֡���£��������Pending���������յ����»ص�
	// �������Complete�����Ժ��ٸ������Service���������Failure�����������
	virtual AsyncResult Update(const float vfElaspedTime);

	// ��ȡ��ʾ���ƣ����������ⲿ��ȡ��ʾ�����ԡ�Log��
	virtual std::wstring GetDisplayName();

	// Ϊ�˵ȴ�t3d��Ⱦ����ȫ��ʼ����ɣ�������UI��ʼ���ᵽ��������
	static void DoRealInitialize(const v8::FunctionCallbackInfo<v8::Value> &vrArgs);

	// ���ⲿ���UI�ڲ��������Ϣ
	void OutputUIInformation(DinoUICStr vstrInformation);

	// ���ƴ���
	void CopyToClipboard(DinoUICStr vstrContent);

	// ճ������
	DinoUICStr GetClipBoardContent(void);
};

#endif