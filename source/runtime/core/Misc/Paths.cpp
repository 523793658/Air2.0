#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "Paths.h"
#include "Misc/Char.h"
#include "Misc/CString.h"
#include "Misc/App.h"
#include "boost/algorithm/string.hpp"
#include "Math/Math.h"
#include "Containers/StringUtil.h"
namespace Air
{
	namespace Paths_Private
	{
		wstring convertRelativePathToFullInternal(wstring&& basePath, wstring&& inPath)
		{
			wstring fullPathed;
			if (Paths::isAbsolute(inPath.c_str()))
			{
				fullPathed = inPath;
			}
			else
			{
				fullPathed = basePath + inPath;
			}

			Paths::normalizeFilename(fullPathed);
			Paths::collapseRelativeDirectories(fullPathed);
			if (fullPathed.length() == 0)
			{
				fullPathed = TEXT("/");
			}
			return fullPathed;
		}
	}

	bool Paths::collapseRelativeDirectories(wstring& inPath)
	{
		const TCHAR parentDir[] = TEXT("/..");
		const int32 parentDirLength = ARRAY_COUNT(parentDir) - 1;

		for (;;)
		{
			if (inPath.empty())
			{
				break;
			}

			if (boost::starts_with(inPath, TEXT("..")) || boost::starts_with(inPath, parentDir))
			{
				return false;
			}

			const int32 index = inPath.find(parentDir);
			if (index == -1)
			{
				break;
			}

			int32 previousSeparatorIndex = index;
			for (;;)
			{
				previousSeparatorIndex = Math::max<int32>(0, StringUtil::find(inPath, TEXT("/"), ESearchCase::CaseSensitive, ESearchDir::FromEnd, previousSeparatorIndex - 1));
				if (previousSeparatorIndex == 0)
				{
					break;
				}
				if ((index - previousSeparatorIndex) > 1 && (inPath[previousSeparatorIndex + 1] != TEXT('.') || inPath[previousSeparatorIndex + 2] != TEXT('/')))
				{
					break;
				}
			}
			int32 colon = StringUtil::find(inPath, TEXT(":"), ESearchCase::CaseSensitive, ESearchDir::FromStart, previousSeparatorIndex);
			if (colon >= 0 && colon < index)
			{
				return false;
			}
			inPath.erase(previousSeparatorIndex, index - previousSeparatorIndex + parentDirLength);
		}
		boost::replace_all(inPath, TEXT("./"), TEXT(""));
		boost::trim(inPath);
		return true;
	}


	wstring Paths::engineDir()
	{
		return PlatformMisc::engineDir();
	}

	bool Paths::directoryExists(const wstring& inPath)
	{
		filesystem::path new_path(inPath);
		return filesystem::exists(new_path);
	}

	wstring Paths::engineConfigDir()
	{
		return Paths::engineDir() + TEXT("Config/");
	}

	wstring Paths::sourceConfigDir()
	{
		return Paths::engineDir() + TEXT("Config/");
	}

	wstring Paths::generateConfigDir()
	{
		return Paths::gameSaveDir() + TEXT("Config/");
	}

	void Paths::normalizeDirectoryName(wstring& inPath)
	{
		boost::algorithm::ireplace_all(inPath, TEXT("\\"), TEXT("/"));
		if (boost::algorithm::iends_with(inPath, TEXT("/")) && !boost::algorithm::iends_with(inPath, TEXT("\\")) && !boost::algorithm::iends_with(inPath, TEXT(":/")))
		{
			inPath[inPath.length() - 1] = 0;
			boost::algorithm::trim(inPath);
		}
		PlatformMisc::normalizePath(inPath);

	}

	bool Paths::shouldSaveToUserDir()
	{
		static bool bShouldSaveToUserDir = App::isInstalled();
		return bShouldSaveToUserDir;
	}

	wstring Paths::gameDir()
	{
		return wstring(PlatformMisc::engineDir());
	}

	wstring Paths::gameUserDir()
	{
		if (shouldSaveToUserDir())
		{
			return nullptr;
		}
		else
		{
			return Paths::gameDir();
		}
	}

	wstring Paths::gameIntermediateDir()
	{
		return gameUserDir() + TEXT("Intermediate/");
	}

	wstring Paths::gameSaveDir()
	{
		static wstring result = Paths::gameUserDir() + TEXT("Saved/");
		return result;
	}

	wstring Paths::getBaseFilename(const wstring& inPath, bool removePath)
	{
		filesystem::path path(inPath);
		if (removePath)
		{
			return path.stem();
		}
		else
		{
			wstring p = path.parent_path();
			p = p + L"/" + path.stem().c_str();
			return p;
		}
	}

	wstring Paths::getCleanFilename(const std::wstring_view inPath)
	{
		filesystem::path path(inPath.data());
		return path.stem();
	}

	wstring Paths::getFilename(const std::wstring_view inPath)
	{
		filesystem::path path(inPath.data());
		return path.filename();
	}

	bool Paths::fileExists(const wstring& path)
	{
		return filesystem::exists(path);
	}

	wstring Paths::convertRelativePathToFull(const wstring& path)
	{
		return Paths_Private::convertRelativePathToFullInternal(PlatformProcess::baseDir(), wstring(path));
	}

	wstring Paths::rootDir()
	{
		return wstring(PlatformMisc::rootDir());
	}
	bool Paths::makePathRelativeTo(wstring& inPath, const TCHAR* inRelativeTo)
	{
		wstring target = Paths::convertRelativePathToFull(inPath);
		wstring source = Paths::convertRelativePathToFull(inRelativeTo);
		source = Paths::getPath(source);
		boost::algorithm::replace_all(source, TEXT("\\"), TEXT("/"));
		boost::algorithm::replace_all(target, TEXT("\\"), TEXT("/"));

		std::vector<wstring> targetArray;
		boost::split(targetArray, target, boost::is_any_of(TEXT("/")));
		std::vector<wstring> sourceArray;
		boost::algorithm::split(sourceArray, source, boost::is_any_of("/"));
		if (targetArray.size() && sourceArray.size())
		{
			if ((targetArray[0][1] == TEXT(':')) && (sourceArray[0][1] == TEXT(':')))
			{
				if (Char::toUpper(targetArray[0][0]) != Char::toUpper(sourceArray[0][0]))
				{
					return false;
				}
			}
		}
		while (targetArray.size() && sourceArray.size() && targetArray[0] == sourceArray[0])
		{
			targetArray.erase(targetArray.begin());
			sourceArray.erase(sourceArray.begin());
		}
		wstring result;
		for (int32 index = 0; index < sourceArray.size(); index++)
		{
			result += TEXT("../");
		}
		for (int32 index = 0; index < targetArray.size(); index++)
		{
			result += targetArray[index];
			if (index + 1 < targetArray.size())
			{
				result += TEXT("/");
			}
		}
		inPath = result;
		return true;
	}

	const wstring & Paths::getRelativePathToRoot()
	{
		struct RelativePathInitializer
		{
			wstring relativePathToRoot;
			RelativePathInitializer()
			{
				wstring rootDirectory = Paths::rootDir();
				wstring baseDirectory = PlatformProcess::baseDir();
				relativePathToRoot = rootDirectory;
				Paths::makePathRelativeTo(relativePathToRoot, baseDirectory.c_str());
				if ((relativePathToRoot.length() > 0) && (boost::algorithm::ends_with(relativePathToRoot, TEXT("/")) == false) && (boost::algorithm::ends_with(relativePathToRoot, TEXT("\\")) == false))
				{
					relativePathToRoot += TEXT("/");
				}
			}
		};
		static RelativePathInitializer StaticInstance;
		return StaticInstance.relativePathToRoot;
	}

	void Paths::normalizeFilename(wstring& inPath)
	{
		boost::replace_all(inPath, TEXT("\\"), TEXT("/"));
		PlatformMisc::normalizePath(inPath);
	}

	void Paths::combineInternal(wstring& outPath, const TCHAR** pathes, int32 numPath)
	{
		BOOST_ASSERT(pathes != nullptr && numPath > 0);
		int32 outStringSize = 0;
		for (int32 i = 0; i < numPath; ++i)
		{
			outStringSize += CString::strlen(pathes[i]) + 1;
		}
		outPath.reserve(outStringSize);
		outPath += pathes[0];
		for (int32 i = 1; i < numPath; ++i)
		{
			outPath /= pathes[i];
		}
	}

	wstring Paths::getAbsolutePath(const wstring& inPath)
	{
		return Paths_Private::convertRelativePathToFullInternal(Paths::rootDir(), wstring(inPath));
	}

	
}