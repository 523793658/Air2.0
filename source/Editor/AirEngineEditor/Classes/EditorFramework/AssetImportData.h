#pragma once
#include "EditorConfig.h"
#include "Misc/Optional.h"
#include "Misc/SecureHash.h"
#include "Misc/DateTime.h"
namespace Air
{
	struct EDITOR_ENGINE_API AssetImportInfo
	{
		struct SourceFile
		{
			SourceFile(wstring inRelativeFilename, const MD5Hash& inFileHash = MD5Hash())
				:mRelativeFilename(inRelativeFilename)
				, mFileHash(inFileHash)
			{

			}

			DateTime mTimestamp;
			wstring mRelativeFilename;
			MD5Hash mFileHash;
		};

		wstring toJson() const;

		static TOptional<AssetImportInfo> fromJson(wstring inJsonString);

		TArray < SourceFile> mSourceFiles;
	};

	class EDITOR_ENGINE_API AssetImportData
	{
	public:

		virtual ~AssetImportData() { }
#if WITH_EDITORONLY_DATA
		AssetImportInfo mSourceData;
#endif
	};
}