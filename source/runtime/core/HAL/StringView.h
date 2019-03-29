#pragma once
#include "CoreType.h"
#if AIR_CXX17_LIBRARY_STRING_VIEW_SUPPORT
#include <string_view>
#else
#include "boost/utility/string_view.hpp"
namespace std
{
	using boost::basic_string_view;
	using boost::string_view;
	using boost::wstring_view;

}


#endif

namespace Air
{
	using std::string_view;
	using std::wstring_view;
}