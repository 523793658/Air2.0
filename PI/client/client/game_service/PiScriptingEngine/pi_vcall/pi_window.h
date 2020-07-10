#ifndef _PI_WINDOW_H_
#define _PI_WINDOW_H_


#include "v8.h"
#include "nan/nan.h"

class WindowBinding
{
public:
	//���window���¼�
	static NAN_METHOD(AddEventListener);
	//���ô��ڴ�С
	static NAN_METHOD(setWindowSize);
	//���ô��ڱ���
	static NAN_METHOD(setWindowTitle);
	//ִ�д�����˸
	static NAN_METHOD(flashWindow);
	//��ȡ���ڴ�Сreturn {w:1,height:1}
	static int getWindowSize();
	//�ı�ȫ��״̬
	static NAN_METHOD(toggleFullScreen);
	//ִ�����
	static NAN_METHOD(setWindowsShow);
	//��ʾ���ش���
	static NAN_METHOD(showMainWindow);
	//���á��������뷨
	static NAN_METHOD(immEnable);
	//���ٴ���
	static NAN_METHOD(destroyWindow);
	//ͨ��Keyֵ��ȡ�����в���
	static NAN_METHOD(getCommandByKey);
	//��Ĭ�������
	static NAN_METHOD(openUrl);
	//��ȡ���������΢����
	static NAN_METHOD(getMachineTime);
	
};


#endif