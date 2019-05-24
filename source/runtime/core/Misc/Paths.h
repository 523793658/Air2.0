#pragma once
#include "CoreType.h"
#include "Containers/String.h"
#include "HAL/StringView.h"
#include "HAL/PlatformProcess.h"

#if AIR_TS_LIBRARY_FILESYSTEM_V3_SUPPORT
#include <experimental/filesystem>
#elif AIR_TS_LIBRARY_FILESYSTEM_V2_SUPPORT
#include <filesystem>
namespace std
{
	namespace experimental
	{
		namespace filesystem = std::tr2::sys;
	}
}

#endif


namespace Air
{
	using namespace std::experimental;

	class CORE_API Paths
	{
	public:
		static wstring engineDir();

		static wstring engineConfigDir();

		static wstring sourceConfigDir();

		static bool directoryExists(const wstring& inPath);

		static wstring getBaseFilename(const wstring& inPath, bool removePath = true);

		static wstring getCleanFilename(const std::wstring_view inPath);

		static wstring getFilename(const std::wstring_view inPath);

		static bool fileExists(const wstring& path);

		static wstring convertRelativePathToFull(const wstring& path);

		static wstring gameIntermediateDir();

		static wstring gameUserDir();

		static bool shouldSaveToUserDir();

		static wstring gameSaveDir();

		static wstring gameDir();

		static wstring generateConfigDir();

		static wstring getAbsolutePath(const wstring& inPath);

		static bool collapseRelativeDirectories(wstring& inPath);

		static FORCEINLINE wstring getPath(const wstring & inPath)
		{
			return filesystem::path(inPath).parent_path();
		}

		static wstring rootDir();

		static const wstring& getRelativePathToRoot();
		
		static bool makePathRelativeTo(wstring& inPath, const TCHAR* inRelativeTo);

		static void makeStandardFilename(wstring& inPath)
		{
			return;
		}
		static void normalizeFilename(wstring& inPath);
		FORCEINLINE static wstring combine(const TCHAR* pathA, const TCHAR* pathB)
		{
			const TCHAR* pathes[] = { pathA, pathB };
			wstring out;
			combineInternal(out, pathes, 2);
			return out;
		}

		FORCEINLINE static wstring combine(const TCHAR* pathA, const TCHAR* pathB, const TCHAR* PathC)
		{
			const TCHAR* pathes[] = { pathA, pathB, PathC };
			wstring out;
			combineInternal(out, pathes, 3);
			return out;
		}

		FORCEINLINE static wstring combine(const TCHAR* pathA, const TCHAR* pathB, const TCHAR* PathC, const TCHAR* pathD)
		{
			const TCHAR* pathes[] = { pathA, pathB, PathC, pathD };
			wstring out;
			combineInternal(out, pathes, 4);
			return out;
		}

		FORCEINLINE static wstring getExtension(const wstring& inPath, bool bIncludeDot = false)
		{
			filesystem::path p(inPath);
			wstring extension = p.extension().wstring();
			if (!bIncludeDot && extension != TEXT(""))
			{
				return extension.substr(1);
			}
			return extension;
		}

		static void normalizeDirectoryName(wstring& inPath);

		static bool isAbsolute(const TCHAR* inPath)
		{
			filesystem::path path(inPath);
			return path.is_absolute();
		}

	protected:
		static void combineInternal(wstring& outPath, const TCHAR** pathes, int32 numPath);
	};
}