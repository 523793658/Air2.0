#include "Containers/String.h"
#include "Serialization/Archive.h"
#include "Misc/CString.h"
namespace Air
{
	Archive& operator<<(Archive& ar, wstring& a)
	{
		const TCHAR* c = a.c_str();
		bool saveUCS2Char = ar.isForcingUnicode() || !CString::isPureAnsi(c);
		int32 num = a.length();
		int32 saveNum = saveUCS2Char ? -num : num;
		ar << saveNum;
		if (saveNum)
		{
			if (saveUCS2Char)
			{
			}
			else
			{
				ar.serialize((void*)a.data(), sizeof(TCHAR) * num);
			}
		}
		return ar;
	}


	Archive& operator<<(Archive& ar, TCHAR* a)
	{
		int32 num = CString::strlen(a);
		ar << num;
		ar.serialize((void*)a, sizeof(TCHAR) * num);
		return ar;
	}

	void pathAppend(wstring& basePath, const TCHAR* str, int32 strLength)
	{
		int32 dataNum = basePath.length();
		if (strLength == 0)
		{
			if (dataNum > 0 && basePath[dataNum - 1] != TEXT('/') && basePath[dataNum - 1] != TEXT('\\'))
			{
				basePath.push_back('/');
			}
		}
		else
		{
			if (dataNum > 0)
			{
				if (dataNum > 0 && basePath[dataNum - 1] != TEXT('/') && basePath[dataNum - 1] != TEXT('\\') && *str != TEXT('/'))
				{
					basePath.push_back('/');
				}
			}
			basePath.reserve(dataNum + strLength + 1);
			basePath.append(str, strLength);

		}
	}
}