#include "SceneTypes.h"
#include "boost/algorithm/string.hpp"
namespace Air
{
	/*
	MP_BaseColor,
		MP_Normal,
		MP_Metallic,
		MP_Specular,
		MP_Roughness,
	*/
#define ADD_MP(mp)	mMap.emplace(TEXT(#mp), mp)

	struct Str2MP
	{
		std::unordered_map<wstring, EMaterialProperty> mMap;

		Str2MP()
		{
			ADD_MP(MP_BaseColor);
			ADD_MP(MP_Normal);
			ADD_MP(MP_Metallic);
			ADD_MP(MP_Specular);
			ADD_MP(MP_Roughness);
		}

		EMaterialProperty find(TCHAR* str)
		{
			auto it = mMap.find(str);
			if (it == mMap.end())
			{
				return MP_Max;
			}
			return it->second;
		}
	};



	EMaterialProperty stringToMaterialProperty(TCHAR* str)
	{
		static Str2MP mMap;
		return mMap.find(str);
	}
}