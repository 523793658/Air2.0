#pragma once

#ifndef DISABLE_DEPRECATION
#if defined(__clang__)
	
#else
#define DEPRECATED(VERSION, MESSAGE) __declspec(deprecated(MESSAGE "Please update your code to the new API before upgrading to the next release, otherwise your project will no longer compile."))

#define PRAGMA_DISABLE_DEPRECATION_WARNINGS \
			__pragma (warning(push)) \
			__pragma (warning(disable:4995)) \
			__pragma (warning(disable:4996))

#define PRAGMA_ENABLE_DEPRECATION_WARNINGS \
			__pragma (warning(pop))
#endif
#endif



