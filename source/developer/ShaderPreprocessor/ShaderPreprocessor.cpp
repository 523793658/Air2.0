#include "ShaderPreprocessor.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Containers/StringConv.h"
#include "PreprocessorPrivate.h"
namespace Air
{
	class McppFileLoader
	{
	public:
		explicit McppFileLoader(const ShaderCompilerInput& inShaderInput)
			:mShaderInput(inShaderInput)
		{
			wstring shaderDir = PlatformProcess::shaderDir();
			mInputShaderFile = shaderDir + TEXT("/") + Paths::getCleanFilename(mShaderInput.mSourceFilename);
			if (Paths::getExtension(mInputShaderFile) != TEXT("hlsl"))
			{
				mInputShaderFile += TEXT(".hlsl");
			}
			wstring inputShaderSource;
			if (loadShaderSourceFile(mShaderInput.mSourceFilename.c_str(), inputShaderSource))
			{
				inputShaderSource = printf(TEXT("%s\n#line 1\n%s"), mShaderInput.mSourceFilePrefix.c_str(), inputShaderSource.c_str());
			}
		}

		file_loader getMcppInterface()
		{
			file_loader loader;
			loader.get_file_contents = getFileContents;
			loader.user_data = (void*)this;
			return loader;
		}

		const wstring& getInputShaderFilename() const
		{
			return mInputShaderFile;
		}

	private:


		typedef TArray<ANSICHAR> ShaderContents;

		static int getFileContents(void* inUserData, const ANSICHAR* inFilename, const ANSICHAR** outContents, size_t* outContentSize)
		{
			McppFileLoader* THis = (McppFileLoader*)inUserData;
			wstring filename = getRelativeShaderFilename(ANSI_TO_TCHAR(inFilename));
			auto cachedContentsIt = THis->mCachedFileContents.find(filename);
			bool finded = false;
			if (cachedContentsIt == THis->mCachedFileContents.end())
			{
				wstring fileContents;
				auto it = THis->mShaderInput.mEnvironment.mIncludeFileNameToContentsMap.find(filename);
				if (it != THis->mShaderInput.mEnvironment.mIncludeFileNameToContentsMap.end())
				{
					fileContents = UTF8_TO_TCHAR(it->second.data());
				}
				else
				{
					loadShaderSourceFile(filename.c_str(), fileContents);
				}
				if (fileContents.length() > 0)
				{
					cachedContentsIt = THis->mCachedFileContents.emplace(filename, stringToArray<ANSICHAR>(fileContents.c_str(), fileContents.length())).first;
					finded = true;
				}
			}
			else
			{
				finded = true;
			}
			if (outContents)
			{
				*outContents = finded ? cachedContentsIt->second.getData() : nullptr;
			}
			if (outContentSize)
			{
				*outContentSize = finded ? cachedContentsIt->second.size() : 0;
			}
			return finded;
		}


		const ShaderCompilerInput& mShaderInput;
		TMap<wstring, ShaderContents> mCachedFileContents;
		wstring mInputShaderFile;
	};

	static void addMcppDefines(wstring& outOptions, const TMap<wstring, wstring>& definitions)
	{
		for (auto& it : definitions)
		{
			outOptions += printf(TEXT(" -D%s=%s"), it.first.c_str(), it.second.c_str());
		}
	}


	bool preprocessShader(wstring& outPreprocessedShader, ShaderCompilerOutput& shaderOutput, const ShaderCompilerInput& shaderInput, const ShaderCompilerDefinitions& additionalDefines)
	{
		if (shaderInput.bSkipPreprocessedCache)
		{
			return FileHelper::loadFileToString(outPreprocessedShader, shaderInput.mSourceFilename.c_str());
		}
		wstring McppOptions;
		wstring McppOutput, McppErrors;
		ANSICHAR* McppOutAnsi;
		ANSICHAR* McppErrAnsi;

		bool bSuccess = false;

		static std::mutex McppMutex;
		std::lock_guard<std::mutex> lock(McppMutex);
		McppFileLoader fileLoader(shaderInput);
		addMcppDefines(McppOptions, shaderInput.mEnvironment.getDefinitions());
		addMcppDefines(McppOptions, additionalDefines.getDefinionMap());

		int32 result = mcpp_run(TCHAR_TO_ANSI(McppOptions.c_str()), TCHAR_TO_ANSI(fileLoader.getInputShaderFilename().c_str()),
			&McppOutAnsi,
			&McppErrAnsi,
			fileLoader.getMcppInterface());
		if (McppOutAnsi != nullptr)
		{
			McppOutput = ANSI_TO_TCHAR(McppOutAnsi);
		}
		if (McppErrAnsi != nullptr)
		{
			McppErrors = ANSI_TO_TCHAR(McppErrAnsi);
		}
		if (parseMcppErrors(shaderOutput.mErrors, McppErrors, true))
		{
			Memory::memswap(&outPreprocessedShader, &McppOutput, sizeof(wstring));
			bSuccess = true;
		}
		return bSuccess;

	}
}