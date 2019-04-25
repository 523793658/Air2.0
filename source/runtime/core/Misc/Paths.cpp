#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "Paths.h"
#include "Misc/Char.h"
#include "Misc/CString.h"
#include "Misc/App.h"
#include "boost/algorithm/string.hpp"

namespace Air
{
	wstring Paths::engineDir()
	{
		return PlatformMisc::engineDir();
	}

	bool Paths::directoryExists(const wstring& inPath)
	{
		filesystem::path new_path(inPath);
		return filesystem::exists(new_path);
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
		return path;
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

	
}