#include "Classes/Engine/EngineBaseTypes.h"
#include "Classes/GameMapsSetting.h"
#include "Containers/Array.h"
namespace Air
{

	UrlConfig URL::mUrlConfig;

	bool URL::bDefaultInitialized = false;

	URL::URL(const TCHAR* localFilename /* = nullptr */)
		:mProtocol(mUrlConfig.mDefaultProtocol)
		,mHost(mUrlConfig.mDefaultHost)
		,mPort(mUrlConfig.mDefaultPort)
		,mOp()
		,mPortal(mUrlConfig.mDefaultPortal)
		,mValid(1)
	{
		if (localFilename)
		{
			mMap = GameMapsSettings::getGameDefaultMap();
		}
	}

	static inline TCHAR* helperStrchr(TCHAR* src, TCHAR A, TCHAR B)
	{
		TCHAR* AA = CString::strchr(src, A);
		TCHAR* BB = CString::strchr(src, B);
		return (AA && (!BB || AA < BB)) ? AA : BB;
	}

	static bool validNetChar(const TCHAR* c)
	{
		if (CString::strchr(c, '?') || CString::strchr(c, '#'))
		{
			return false;
		}
		return true;
	}

	URL::URL(URL* base, const TCHAR* textURL, ETravelType type)
		:mProtocol(mUrlConfig.mDefaultProtocol)
		,mHost(mUrlConfig.mDefaultHost)
		,mPort(mUrlConfig.mDefaultPort)
		,mMap(GameMapsSettings::getGameDefaultMap())
		,mOp()
		,mPortal(mUrlConfig.mDefaultPortal)
		,mValid(1)
	{
		BOOST_ASSERT(textURL);
		if (!bDefaultInitialized)
		{
			URL::staticInit();
			mProtocol = mUrlConfig.mDefaultProtocol;
			mHost = mUrlConfig.mDefaultHost;
			mPort = mUrlConfig.mDefaultPort;
			mPortal = mUrlConfig.mDefaultPortal;
		}
		const int32 URLLength = CString::strlen(textURL);
		TCHAR* tempURL = new TCHAR[URLLength + 1];
		TCHAR* url = tempURL;
		CString::strcpy(tempURL, URLLength + 1, textURL);
		if (type == TRAVEL_Relative)
		{
			BOOST_ASSERT(base);
			mProtocol = base->mProtocol;
			mHost = base->mHost;
			mMap = base->mMap;
			mPortal = base->mPortal;
			mPort = base->mPort;
		}
		if (type == TRAVEL_Relative || type == TRAVEL_Partial)
		{
			BOOST_ASSERT(base);
			for (int32 i = 0; i < base->mOp.size(); i++)
			{
				new(mOp)wstring(base->mOp[i]);
			}
		}
		while (*url == ' ')
		{
			url++;
		}
		TCHAR* s = helperStrchr(url, '?', '#');
		if (s)
		{
			TCHAR optionChar = *s, nextOptionChar = 0;
			*(s++) = 0;
			do 
			{
				TCHAR*t = helperStrchr(s, '?', '#');
				if (t)
				{
					nextOptionChar = *t;
					*(t++) = 0;
				}
				if (!validNetChar(s))
				{
					*this = URL();
					mValid = 0;
					break;
				}
				if (optionChar == '?')
				{
					if (s && s[0] == '-')
					{
						s++;
						removeOption(s);
					} 
					else
					{
						addOption(s);
					}
				}
				else
				{
					mPortal = s;
				}
				s = t;
				optionChar = nextOptionChar;
			} while (s);
		}
		if (valid == 1)
		{
			bool farHost = 0;
			bool farMap = 0;
			if (CString::strlen(url) > 2 && ((url[0] != '[' && url[1] == ':') || (url[0] == '/')))
			{
				mProtocol = mUrlConfig.mDefaultProtocol;
				mMap = url;
				mPortal = mUrlConfig.mDefaultPortal;
				url = nullptr;
				farHost = 1;
				farMap = 1;
				mHost = TEXT("");
			}
			else
			{
				const TCHAR* squareBracket = CString::strchr(url, '[');
				if ((CString::strchr(url, ':') != NULL) && (CString::strchr(url, ':') > url + 1) && (!squareBracket || (CString::strchr(url, ':') < squareBracket)) && (CString::strchr(url, '.') == NULL || CString::strchr(url, ':') < CString::strchr(url, '.')))
				{
					TCHAR* ss = url;
					url = CString::strchr(url, ':');
					*(url++) = 0;
					mProtocol = ss;
				}

				if (*url == '/' && *(url + 1) == '/')
				{
					url += 2;
					farHost = 1;
					mHost = TEXT("");
				}

				const TCHAR* dot = CString::strchr(url, '.');
				//const int32 extLen = 
			}
		}
	}

	void URL::staticInit()
	{
		mUrlConfig.init();
		bDefaultInitialized = true;
	}

	void URL::staticExit()
	{
		mUrlConfig.reset();
		bDefaultInitialized = false;
	}

	wstring URL::toString(bool bFullyQualified /* = false */) const
	{
		wstring result;
		if ((mProtocol != mUrlConfig.mDefaultProtocol) || bFullyQualified)
		{
			result += mProtocol;
			result += TEXT(":");
			if (mHost != mUrlConfig.mDefaultHost)
			{
				result += TEXT("//");
			}
		}

		if ((mHost != mUrlConfig.mDefaultHost) || (mPort != mUrlConfig.mDefaultPort))
		{
			result += mHost;
			if (mPort != mUrlConfig.mDefaultPort)
			{
				result += TEXT(":");
				result += mPort;
			}
			result += TEXT("/");
		}
		return result;
	}


	void URL::loadURLConfig(const TCHAR* section, const wstring& filename /* = GGameIni */)
	{
		TArray<wstring> options;
		GConfig->getSection(section, options, filename);
		for (int32 i = 0; i < options.size(); i++)
		{
			addOption(options[i].c_str());
		}
	}

	bool URL::isLocalInternal() const
	{
		return isInternal() && mHost.length() == 0;
	}

	bool URL::isInternal() const
	{
		return mProtocol == mUrlConfig.mDefaultProtocol;
	}

	void URL::addOption(const TCHAR* str)
	{

	}

	void URL::removeOption(const TCHAR* key, const TCHAR* section /* = nullptr */, const wstring& filename /* = GGameIni */)
	{

	}
}