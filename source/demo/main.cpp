#include "Containers/Array.h"
#include "DemoType.h"
#include <iostream>
#include "ApplicationManager.h"
#include "LaunchMininal.h"
#include "Containers/StringConv.h"

int main(int argc, char* argv[])
{
	TCHAR *tc = TEXT("asdfasdf����Ͱ�ɪ��");
	TCHARToUTF8 utf8String(tc);
	char* c = (char*)utf8String.get();
}