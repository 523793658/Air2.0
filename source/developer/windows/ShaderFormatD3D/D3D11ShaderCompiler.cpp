#include "ShaderFormatD3D.h"
#include "ShaderCore.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformString.h"
#include "HAL/FileManager.h"
#include "dxsdk/Include/d3dcompiler.h"
#include "Containers/StringConv.h"
#include "Containers/IndirectArray.h"
#include "Containers/BitArray.h"
#include "ConstantBuffer.h"
#include "ShaderCompilerCommon.h"
#include "Serialization/MemoryWriter.h"
#include "boost/algorithm/string.hpp"
#include "../D3D11RHI/D3D11Shader.h"
#include "ShaderPreprocessor.h"
#include "d3d11.h"
#include "d3d11shader.h"
namespace Air
{
#ifdef _DEBUG
#define DEBUG_SHADER 1
#else
#define DEBUG_SHADER 0
#endif
	static int32 GD3DAllowRemoveUnused = 0;

#define SHADER_OPTIMIZATION_LEVEL_MASk (D3D10_SHADER_OPTIMIZATION_LEVEL0 | D3D10_SHADER_OPTIMIZATION_LEVEL1 | D3D10_SHADER_OPTIMIZATION_LEVEL2 |D3D10_SHADER_OPTIMIZATION_LEVEL3)

	static const TCHAR* getShaderProfileName(ShaderTarget target)
	{
		if (target.mPlatform == SP_PCD3D_SM5)
		{
			BOOST_ASSERT(target.mFrequency == SF_Vertex ||
				target.mFrequency == SF_Pixel ||
				target.mFrequency == SF_Domain ||
				target.mFrequency == SF_Hull ||
				target.mFrequency == SF_Geometry ||
				target.mFrequency == SF_Compute);
			switch (target.mFrequency)
			{
			case SF_Pixel:
				return TEXT("ps_5_0");
			case SF_Domain:
				return TEXT("ds_5_0");
			case SF_Hull:
				return TEXT("hs_5_0");
			case SF_Geometry:
				return TEXT("gs_5_0");
			case SF_Compute:
				return TEXT("cs_5_0");
			case SF_Vertex:
				return TEXT("vs_5_0");
			}
		}
		else if (target.mPlatform == SP_PCD3D_SM4 || target.mPlatform == SP_PCD3D_ES2 || target.mPlatform == SP_PCD3D_ES3_1)
		{
			BOOST_ASSERT(target.mFrequency == SF_Vertex || target.mFrequency == SF_Pixel || target.mFrequency == SF_Geometry);
			switch (target.mFrequency)
			{
			case SF_Pixel:
				return TEXT("ps_4_0");
			case SF_Vertex:
				return TEXT("vs_4_0");
			case SF_Geometry:
				return TEXT("gs_4_0");
			}
		}
		return nullptr;
	}

	static uint32 translateCompilerFlagD3D11(ECompilerFlags compilerFlag)
	{
		switch (compilerFlag)
		{
		case CFLAG_PreferFlowControl:
			return D3D10_SHADER_PREFER_FLOW_CONTROL;
		case CFLAG_AvoidFlowControl:
			return D3D10_SHADER_AVOID_FLOW_CONTROL;
		default:
			return 0;
		}
	}

	static wstring d3d11CreateShaderCompileCommandLine(const wstring& shaderPath, const TCHAR* entryFunction, const TCHAR* shaderProfile, uint32 compileFlags, ShaderCompilerOutput& output)
	{
		wstring FXCCommandline = wstring(TEXT("\"%DXSDK_DIR%\\Utilities\\bin\\x86\\fxc\" ")) + shaderPath;
		FXCCommandline += wstring(TEXT(" /E ")) + entryFunction;

		if (compileFlags & D3D10_SHADER_PREFER_FLOW_CONTROL)
		{
			compileFlags &= ~D3D10_SHADER_PREFER_FLOW_CONTROL;
			FXCCommandline += wstring(TEXT(" /Gfp"));
		}

		if (compileFlags & D3D10_SHADER_DEBUG)
		{
			compileFlags &= ~D3D10_SHADER_DEBUG;
			FXCCommandline += wstring(TEXT(" /Zi"));
		}

		if (compileFlags & D3D10_SHADER_SKIP_OPTIMIZATION)
		{
			compileFlags &= ~D3D10_SHADER_SKIP_OPTIMIZATION;
			FXCCommandline += wstring(TEXT(" /Od"));
		}
		if (compileFlags & D3D10_SHADER_SKIP_VALIDATION)
		{
			compileFlags &= ~D3D10_SHADER_SKIP_VALIDATION;
			FXCCommandline += wstring(TEXT(" /Vd"));
		}

		if (compileFlags & D3D10_SHADER_AVOID_FLOW_CONTROL)
		{
			compileFlags &= ~D3D10_SHADER_AVOID_FLOW_CONTROL;
			FXCCommandline += wstring(TEXT(" /Gfa"));
		}

		if (compileFlags & D3D10_SHADER_PACK_MATRIX_ROW_MAJOR)
		{
			compileFlags &= ~D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;
			FXCCommandline += wstring(TEXT(" /Zpr"));
		}

		if (compileFlags & D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY)
		{
			compileFlags &= ~D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY;
			FXCCommandline += wstring(TEXT(" /Gec"));
		}

		switch (compileFlags& SHADER_OPTIMIZATION_LEVEL_MASk)
		{
		case D3D10_SHADER_OPTIMIZATION_LEVEL2:
			compileFlags &= ~D3D10_SHADER_OPTIMIZATION_LEVEL2;
			FXCCommandline += wstring(TEXT(" /02"));
			break;
		case D3D10_SHADER_OPTIMIZATION_LEVEL3:
			compileFlags &= ~D3D10_SHADER_OPTIMIZATION_LEVEL3;
			FXCCommandline += wstring(TEXT(" /03"));
			break;
		case D3D10_SHADER_OPTIMIZATION_LEVEL1:
			compileFlags &= ~D3D10_SHADER_OPTIMIZATION_LEVEL1;
			FXCCommandline += wstring(TEXT(" /01"));
			break;
		case D3D10_SHADER_OPTIMIZATION_LEVEL0:
			compileFlags &= ~D3D10_SHADER_OPTIMIZATION_LEVEL0;
			break;
		default:
			output.mErrors.emplace(TEXT("Unknown D3D10 optimization level"));
			break;
		}
		BOOST_ASSERT(compileFlags == 0);
		FXCCommandline += wstring(TEXT(" /T ")) + shaderProfile;
		FXCCommandline += TEXT(" /Ni");
		if (Paths::getExtension(shaderPath) == TEXT(".hlsl"))
		{
			FXCCommandline += printf(TEXT(" /Fc%sd3dasm"), shaderPath.substr(0, shaderPath.length() - 3));
		}
		FXCCommandline += wstring(TEXT(" \r\n pause"));
		return FXCCommandline;
	}

	wstring createShaderCompilerWorkerDirectCommandLine(const ShaderCompilerInput& input)
	{
		wstring text(TEXT("-directcompile -format="));
		text += input.mShaderFormat;
		text += TEXT(" -entry=");
		text += input.mEntryPointName;
		switch (input.mTarget.mFrequency)
		{
		case SF_Vertex:
			text += TEXT(" -vs");
			break;
		case SF_Domain:
			text += TEXT(" -ds");
			break;
		case SF_Hull:
			text += TEXT(" -hs");
			break;
		case SF_Geometry:
			text += TEXT(" -gs");
			break;
		case SF_Pixel:
			text += TEXT(" -ps");
			break;
		case SF_Compute:
			text += TEXT(" -cs");
			break;
		default:
			BOOST_ASSERT(false);
			break;
		}
		if (input.bCompilingForShaderPipeline)
		{
			text += TEXT(" -pipeline");
		}
		if (input.bIncludeUsedOutputs)
		{
			text += TEXT(" -usedoutputs");
			for (int32 index = 0; index < input.mUsedOutputs.size(); ++index)
			{
				if (index != 0)
				{
					text += TEXT("+");
				}
				text += input.mUsedOutputs[index];

			}
		}
		text += TEXT(" ");
		text += input.mDumpDebugInfoPath + input.mSourceFilename + TEXT(".hlsl");
		uint64 cFlags = 0;
		for (int32 index = 0; index < input.mEnvironment.mCompilerFlags.size(); ++index)
		{
			cFlags = cFlags | ((uint64)1 << (uint64)input.mEnvironment.mCompilerFlags[index]);
		}
		if (cFlags)
		{
			text += TEXT(" -cflags=");
			text += printf(TEXT("%llu"), cFlags);
		}
		return text;
	}

	pD3DCompile getD3DCompileFunc(const wstring& newCompilerPath)
	{
		static wstring currentCompiler;
		static HMODULE compilerDll = 0;
		if (currentCompiler != newCompilerPath.c_str())
		{
			currentCompiler = newCompilerPath.c_str();
			if (compilerDll)
			{
				FreeLibrary(compilerDll);
				compilerDll = nullptr;
			}
			if (currentCompiler.length())
			{
				compilerDll = LoadLibrary(currentCompiler.c_str());
			}
			if (!compilerDll && newCompilerPath.length())
			{
				return 0;
			}

		}
		if (compilerDll)
		{
			return (pD3DCompile)(void*)GetProcAddress(compilerDll, "D3DCompile");
		}
		return nullptr;
		//return &D3DCompile;
	}

	HRESULT D3DCompileWrapper(
		pD3DCompile	D3DCompileFunc,
		bool&	bException,
		LPCVOID	pSrcData,
		SIZE_T	srcDataSize,
		LPCSTR	pFileName,
		CONST D3D_SHADER_MACRO* pDefines,
		ID3DInclude*	pInclude,
		LPCSTR	pEntryPoint,
		LPCSTR	pTarget,
		uint32	Flags1,
		uint32	flags2,
		ID3DBlob**	ppCode,
		ID3DBlob**	ppErrorMsgs
	)
	{
#if !PLATFORM_SEH_EXCEPTIONS_DISABLED
		__try
#endif
		{
			return D3DCompileFunc(pSrcData,
				srcDataSize,
				pFileName,
				pDefines,
				pInclude,
				pEntryPoint,
				pTarget,
				Flags1,
				flags2,
				ppCode,
				ppErrorMsgs);
		}
#if !PLATFORM_SEH_EXCEPTIONS_DISABLED
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			bException = true;
			return E_FAIL;
		}
#endif
	}

	static void D3D11FilterShaderCompileWarnings(const wstring& compileWarnings, TArray<wstring>& filteredWarnings)
	{
		std::vector<wstring> waringArray;
		wstring outWarningString = TEXT("");
		boost::split(waringArray, compileWarnings, boost::is_any_of(TEXT("\n")));
		for (int32 waringIndex = 0; waringIndex < waringArray.size(); waringIndex++)
		{
			if (!boost::contains(waringArray[waringIndex], TEXT("X3557")) && !boost::contains(waringArray[waringIndex], TEXT("X3205")))
			{
				filteredWarnings.addUnique(waringArray[waringIndex]);
			}
		}
	}
	static int32 GBreakpint = 0;

	void buildResourceTableMapping(
		const TMap<wstring, ResourceTableEntry>& resourceTableMap,
		const TMap<wstring, uint32>& resourceTableLayoutHashes,
		TBitArray<>& usedConstantBufferSlots,
		ShaderParameterMap& parameterMap,
		ShaderCompilerResourceTable& outSRT
	)
	{
		BOOST_ASSERT(outSRT.mResourceTableBits == 0);
		BOOST_ASSERT(outSRT.mResourceTableLayoutHashes.size() == 0);
		int32 maxBoundResourceTable = -1;
		TArray<uint32> mResourceTableSRVs;
		TArray<uint32> mResourceTableSamplerStates;
		TArray<uint32> mResourceTableUAVs;
		for (auto mapIt = resourceTableMap.begin(); mapIt != resourceTableMap.end(); mapIt++)
		{
			const wstring& name = mapIt->first;
			const ResourceTableEntry& entry = mapIt->second;
			uint16 bufferIndex, baseIndex, size;
			if (parameterMap.findParameterAllocation(name.c_str(), bufferIndex, baseIndex, size))
			{
				parameterMap.removeParameterAllocation(name.c_str());
				uint16 constantBufferIndex = INDEX_NONE, CBBaseIndex, CBSize;
				if (parameterMap.findParameterAllocation(entry.mConstantBufferName.c_str(), constantBufferIndex, CBBaseIndex, CBSize) == false)
				{
					constantBufferIndex = usedConstantBufferSlots.findAndSetFirstZeroBit();
					parameterMap.addParameterAllocation(entry.mConstantBufferName.c_str(), constantBufferIndex, 0, 0);
				}
				outSRT.mResourceTableBits |= (1 << constantBufferIndex);
				maxBoundResourceTable = Math::max<int32>(maxBoundResourceTable, (int32)constantBufferIndex);
				while (outSRT.mResourceTableLayoutHashes.size() <= maxBoundResourceTable)
				{
					outSRT.mResourceTableLayoutHashes.add(0);
				}
				outSRT.mResourceTableLayoutHashes[constantBufferIndex] = resourceTableLayoutHashes.findChecked(entry.mConstantBufferName);
				auto resourceMap = RHIResourceTableEntry::create(constantBufferIndex, entry.mResourceIndex, baseIndex);
				switch (entry.mType)
				{
				case CBMT_TEXTURE:
					outSRT.mTextureMap.add(resourceMap);
					break;
				case CBMT_SAMPLER:
					outSRT.mSamplerMap.add(resourceMap);
					break;
				case CBMT_SRV:
					outSRT.mShaderResourceViewMap.add(resourceMap);
					break;
				case CBMT_UAV:
					outSRT.mUnorderedAccessViewMap.add(resourceMap);
					break;
				default:
					BOOST_ASSERT(false);
					break;
				}
			}
		}
		maxBoundResourceTable = Math::max(0, maxBoundResourceTable);
		outSRT.mMaxBoundResourceTable = maxBoundResourceTable;
	}

	static bool compileAndProcessD3DShader(wstring& proprocessedShaderResource, const wstring& compilerPath, uint32 compileFlags, const ShaderCompilerInput& input, wstring& entryPointName, const TCHAR* shaderProfile, bool bProcessingSecondTime, TArray<wstring>& filterdErrors, ShaderCompilerOutput& output)
	{
		auto ansiSourceFile = stringCast<ANSICHAR>(proprocessedShaderResource.c_str());
		if (input.mDumpDebugInfoPath.length() > 0 && IFileManager::get().directoryExist(input.mDumpDebugInfoPath.c_str()))
		{
			Archive* filewriter = IFileManager::get().createFileWriter((input.mDumpDebugInfoPath + input.mSourceFilename + TEXT(".hlsl")).c_str());
			if (filewriter)
			{
				filewriter->serialize((ANSICHAR*)ansiSourceFile.get(), ansiSourceFile.length());
				filewriter->close();
				delete filewriter;
			}

			const wstring batchFileContents = d3d11CreateShaderCompileCommandLine((input.mSourceFilename + TEXT(".hlsl")), entryPointName.c_str(), shaderProfile, compileFlags, output);
			FileHelper::saveStringToFile(batchFileContents, (input.mDumpDebugInfoPath + TEXT("CompileD3D.bat")).c_str());
			if (input.bGenerateDirectCompileFile)
			{
				FileHelper::saveStringToFile(createShaderCompilerWorkerDirectCommandLine(input), (input.mDumpDebugInfoPath + TEXT("DirectCompile.txt")).c_str());
			}
		}
		TRefCountPtr<ID3DBlob> shader;
		TRefCountPtr<ID3DBlob> Errors;

		HRESULT result;
		pD3DCompile D3DCompileFunc = getD3DCompileFunc(compilerPath);
		if (D3DCompileFunc)
		{
			bool bException = false;
			result = D3DCompileWrapper(D3DCompileFunc,
				bException,
				ansiSourceFile.get(),
				ansiSourceFile.length(),
				TCHAR_TO_ANSI(input.mSourceFilename.c_str()),
				NULL,
				NULL,
				TCHAR_TO_ANSI(entryPointName.c_str()),
				TCHAR_TO_ANSI(shaderProfile),
				compileFlags,
				0,
				shader.getInitReference(),
				Errors.getInitReference());
			if (bException)
			{
				filterdErrors.add(TEXT("D3DCompile exception"));
			}
		}
		else
		{
			filterdErrors.add(printf(TEXT("couldn't find shader compiler: %s"), compilerPath.c_str()));
			result = E_FAIL;
		}

		void* errorBuffer = Errors ? Errors->GetBufferPointer() : nullptr;
		if (errorBuffer)
		{
			D3D11FilterShaderCompileWarnings(ANSI_TO_TCHAR(errorBuffer), filterdErrors);
		}

		if (SUCCEEDED(result))
		{
			int32 numInterpolants = 0;
			TindirectArray<wstring> interpolantNames;
			TArray<wstring> shaderInputs;
			if (SUCCEEDED(result))
			{
				output.bSucceeded = true;
				ID3D11ShaderReflection* reflector = nullptr;
				result = D3DReflect(shader->GetBufferPointer(), shader->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector);
				if (FAILED(result))
				{
					AIR_LOG(LogD3D11ShaderCompiler, Fatal, TEXT("D3DReflect failed: result = %08x"), result);
				}
				D3D11_SHADER_DESC shaderDesc;
				reflector->GetDesc(&shaderDesc);
				bool bGlobalConstantBufferUsed = false;
				uint32 numSamplers = 0;
				uint32 numSRVs = 0;
				uint32 numCBs = 0;
				uint32 numUAVs = 0;
				TArray<wstring>	constantBufferNames;
				TArray<wstring> shaderOutputs;
				TBitArray<> usedConstantBufferSlots;
				usedConstantBufferSlots.init(false, 32);
				if (input.mTarget.mFrequency == SF_Vertex)
				{
					for (uint32 index = 0; index < shaderDesc.OutputParameters; ++index)
					{
						D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
						reflector->GetOutputParameterDesc(index, &paramDesc);
						if (paramDesc.SystemValueType == D3D_NAME_UNDEFINED && paramDesc.Mask != 0)
						{
							++numInterpolants;
							new(interpolantNames)wstring(printf(TEXT("%s%d"), ANSI_TO_TCHAR(paramDesc.SemanticName), paramDesc.SemanticIndex));
							shaderOutputs.add(interpolantNames.last().c_str());
						}
					}
				}
				else if (input.mTarget.mFrequency == SF_Pixel)
				{
					if (GD3DAllowRemoveUnused != 0 && input.bCompilingForShaderPipeline)
					{
						++GBreakpint;
					}
					bool bFoundUnused = false;
					for (uint32 index = 0; index < shaderDesc.InputParameters; index++)
					{
						D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
						reflector->GetInputParameterDesc(index, &paramDesc);
						if (paramDesc.SystemValueType == D3D_NAME_UNDEFINED)
						{
							if (paramDesc.ReadWriteMask != 0)
							{
								wstring semanticName = ANSI_TO_TCHAR(paramDesc.SemanticName);
								shaderInputs.addUnique(semanticName);
								wstring semanticIndexName = printf(TEXT("%s%d"), semanticName.c_str(), paramDesc.SemanticIndex);
								shaderInputs.addUnique(semanticIndexName);
								shaderInputs.addUnique(semanticName + TEXT("_centroid"));
								shaderInputs.addUnique(semanticIndexName + TEXT("_centroid"));
							}
							else
							{
								bFoundUnused = true;

							}
						}
						else
						{
							shaderInputs.addUnique(wstring(ANSI_TO_TCHAR(paramDesc.SemanticName)));
						}
					}
					if (GD3DAllowRemoveUnused &&  input.bCompilingForShaderPipeline && bFoundUnused)
					{

					}
				}
				for (uint32 resourceIndex = 0; resourceIndex < shaderDesc.BoundResources; resourceIndex++)
				{
					D3D11_SHADER_INPUT_BIND_DESC bindDesc;
					reflector->GetResourceBindingDesc(resourceIndex, &bindDesc);
					if (bindDesc.Type == D3D_SIT_CBUFFER || bindDesc.Type == D3D_SIT_TBUFFER)
					{
						const uint32 CBIndex = bindDesc.BindPoint;
						ID3D11ShaderReflectionConstantBuffer* constantBuffer = reflector->GetConstantBufferByName(bindDesc.Name);
						D3D11_SHADER_BUFFER_DESC cbDesc;
						constantBuffer->GetDesc(&cbDesc);
						bool bGlobalCB = (CStringAnsi::strcmp(cbDesc.Name, "$Globals") == 0);
						if (bGlobalCB)
						{
							for (uint32 constantIndex = 0; constantIndex < cbDesc.Variables; constantIndex++)
							{
								ID3D11ShaderReflectionVariable* variable = constantBuffer->GetVariableByIndex(constantIndex);
								D3D11_SHADER_VARIABLE_DESC variableDesc;
								variable->GetDesc(&variableDesc);
								if (variableDesc.uFlags & D3D_SVF_USED)
								{
									bGlobalConstantBufferUsed = true;
									output.mParameterMap.addParameterAllocation(ANSI_TO_TCHAR(variableDesc.Name), CBIndex, variableDesc.StartOffset, variableDesc.Size);
									usedConstantBufferSlots[CBIndex] = true;
								}
							}
						}
						else
						{
							output.mParameterMap.addParameterAllocation(ANSI_TO_TCHAR(cbDesc.Name), CBIndex, 0, 0);
							usedConstantBufferSlots[CBIndex] = true;
							if (constantBufferNames.size() <= (int32)CBIndex)
							{
								constantBufferNames.addDefaulted(CBIndex - constantBufferNames.size() + 1);
							}
							constantBufferNames[CBIndex] = ANSI_TO_TCHAR(cbDesc.Name);
						}
						numCBs = Math::max(numCBs, bindDesc.BindPoint + bindDesc.BindCount);

					}
					else if (bindDesc.Type == D3D_SIT_TEXTURE || bindDesc.Type == D3D_SIT_SAMPLER)
					{
						TCHAR officialName[1024];
						uint32 bindCount = bindDesc.BindCount;
						CString::strcpy(officialName, ANSI_TO_TCHAR(bindDesc.Name));
						if (input.mTarget.mPlatform == SP_PCD3D_SM5)
						{
							bindCount = 1;
							TCHAR* bracketLocation = CString::strchr(officialName, TEXT('['));
							if (bracketLocation)
							{
								*bracketLocation = 0;
								const int32 numCharactersBeforeArray = bracketLocation - officialName;
								while (resourceIndex + 1 < shaderDesc.BoundResources)
								{
									D3D11_SHADER_INPUT_BIND_DESC bindDesc2;
									reflector->GetResourceBindingDesc(resourceIndex + 1, &bindDesc2);
									if (bindDesc2.Type == bindDesc.Type && CStringAnsi::strncmp(bindDesc.Name, bindDesc2.Name, numCharactersBeforeArray) == 0)
									{
										bindCount++;
										resourceIndex++;
									}
									else
									{
										break;
									}
								}
							}
						}
						if (bindDesc.Type == D3D_SIT_SAMPLER)
						{
							numSamplers = Math::max(numSamplers, bindDesc.BindPoint + bindDesc.BindCount);
						}
						else if (bindDesc.Type == D3D_SIT_TEXTURE)
						{
							numSRVs = Math::max(numSRVs, bindDesc.BindPoint + bindDesc.BindCount);
						}
						output.mParameterMap.addParameterAllocation(officialName, 0, bindDesc.BindPoint, bindCount);
					}
					else if (bindDesc.Type == D3D11_SIT_UAV_RWTYPED || bindDesc.Type == D3D11_SIT_UAV_RWSTRUCTURED || bindDesc.Type == D3D11_SIT_UAV_RWBYTEADDRESS || bindDesc.Type == D3D11_SIT_UAV_RWSTRUCTURED_WITH_COUNTER || bindDesc.Type == D3D11_SIT_UAV_APPEND_STRUCTURED)
					{
						TCHAR officialName[1024];
						CString::strcpy(officialName, ANSI_TO_TCHAR(bindDesc.Name));
						output.mParameterMap.addParameterAllocation(officialName, 0, bindDesc.BindPoint, 1);
						numUAVs = Math::max(numUAVs, bindDesc.BindPoint + bindDesc.BindCount);
					}
					else if (bindDesc.Type == D3D11_SIT_STRUCTURED || bindDesc.Type == D3D11_SIT_BYTEADDRESS)
					{
						TCHAR officialName[1024];
						CString::strcpy(officialName, ANSI_TO_TCHAR(bindDesc.Name));
						output.mParameterMap.addParameterAllocation(officialName, 0, bindDesc.BindPoint, 1);
						numSRVs = Math::max(numSRVs, bindDesc.BindPoint + bindDesc.BindCount);
					}
				}
				TRefCountPtr<ID3DBlob> compressedData;
				if (input.mEnvironment.mCompilerFlags.contains(CFLAG_KeepDebugInfo) || DEBUG_SHADER)
				{
					compressedData = shader;
				}
				else
				{
					D3D_SHADER_DATA shaderData;
					shaderData.pBytecode = shader->GetBufferPointer();
					shaderData.BytecodeLength = shader->GetBufferSize();
					result = D3DStripShader(shader->GetBufferPointer(), shader->GetBufferSize(), D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO | D3DCOMPILER_STRIP_TEST_BLOBS, compressedData.getInitReference());
					if (FAILED(result))
					{
						AIR_LOG(LogD3D11ShaderCompiler, Fatal, TEXT("D3DStripShader failed: result = %08x"), result);
					}
				}
				D3D11ShaderResourceTable srt;
				TArray<uint8> constantBufferNameBytes;
				{

					ShaderCompilerResourceTable genericSRT;
					buildResourceTableMapping(input.mEnvironment.mResourceTableMap, input.mEnvironment.mResourceTableLayoutHashes, usedConstantBufferSlots, output.mParameterMap, genericSRT);
					if (constantBufferNames.size() < genericSRT.mResourceTableLayoutHashes.size())
					{
						constantBufferNames.addDefaulted(genericSRT.mResourceTableLayoutHashes.size() - constantBufferNames.size() +1);

					}
					for (int32 index = 0; index < genericSRT.mResourceTableLayoutHashes.size(); index++)
					{
						if (genericSRT.mResourceTableLayoutHashes[index] != 0 && constantBufferNames[index].length() == 0)
						{
							auto* name = input.mEnvironment.mResourceTableLayoutHashes.findKey(genericSRT.mResourceTableLayoutHashes[index]);
							BOOST_ASSERT(name);
							constantBufferNames[index] = *name;
						}
					}
					MemoryWriter constantBufferNameWriter(constantBufferNameBytes);
					constantBufferNameWriter << constantBufferNames;
					srt.mResourceTableBits = genericSRT.mResourceTableBits;
					srt.mResourceTableLayoutHashes = genericSRT.mResourceTableLayoutHashes;
					buildResourceTableTokenStream(genericSRT.mTextureMap, genericSRT.mMaxBoundResourceTable, srt.mTextureMap);
					buildResourceTableTokenStream(genericSRT.mShaderResourceViewMap, genericSRT.mMaxBoundResourceTable, srt.mShaderResourceViewMap);
					buildResourceTableTokenStream(genericSRT.mSamplerMap, genericSRT.mMaxBoundResourceTable, srt.mSamplerMap);
					buildResourceTableTokenStream(genericSRT.mUnorderedAccessViewMap, genericSRT.mMaxBoundResourceTable, srt.mUnorderedAccessViewMap);
				}
				MemoryWriter ar(output.mShaderCode.getWriteAccess(), true);
				ar << srt;
				ar.serialize(compressedData->GetBufferPointer(), compressedData->GetBufferSize());
				{
					ShaderCodePackedResourceCounts packedResourceCount = { bGlobalConstantBufferUsed, numSamplers, numSRVs, numCBs, numUAVs };
					output.mShaderCode.addOptionalData(packedResourceCount);
					output.mShaderCode.addOptionalData('u', constantBufferNameBytes.getData(), constantBufferNameBytes.size());

				}
				output.mShaderCode.addOptionalData('n', TCHAR_TO_UTF8(input.generateShaderName().c_str()));

				output.mNumInstructions = shaderDesc.InstructionCount;
				output.mNumTextureSamplers = numSamplers;
				reflector->Release();
				output.mTarget = input.mTarget;
			}
			if (input.mTarget.mPlatform == SP_PCD3D_ES2)
			{
				if (output.mNumTextureSamplers > 8)
				{
					result = E_FAIL;
					output.bSucceeded = false;
				}
				else if (false && numInterpolants > 8)
				{

				}
			}
		}
		else
		{
			++GBreakpint;
		}
		return SUCCEEDED(result);
	}

	void compileD3D11Shader(const ShaderCompilerInput& input, ShaderCompilerOutput& output, ShaderCompilerDefinitions& additionalDefines, const wstring& workingDirectory)
	{
		wstring preprocessedShaderSource;
		wstring compilerPath;
		const TCHAR* shaderProfile = getShaderProfileName(input.mTarget);
		if (!shaderProfile)
		{
			output.mErrors.add(ShaderCompilerError(TEXT("Unrecognized shader frequency")));
			return;
		}

		additionalDefines.setDefine(TEXT("COMPILER_HLSL"), 1);
		if (input.bSkipPreprocessedCache)
		{
			FileHelper::loadFileToString(preprocessedShaderSource, input.mSourceFilename.c_str());
		}
		else
		{
			if (preprocessShader(preprocessedShaderSource, output, input, additionalDefines) != true)
			{
				return;
			}
		}
		GD3DAllowRemoveUnused = input.mEnvironment.mCompilerFlags.contains(CFLAG_ForceRemoveUnusedInterpolators) ? 1 : 0;
		wstring entryPointName = input.mEntryPointName;

		output.bFailedRemovingUnused = false;
		//if (GD3DAllowRemoveUnused == 1 && input.mTarget.mFrequency == SF_Vertex && input.bCompilingForShaderPipeline)
		//{
		//	TArray<wstring> usedOutputs = input.mUsedOutputs;
		//	usedOutputs.addUnique(TEXT("SV_POSITION"));
		//	TArray<wstring> errors;
		//	//if(!removeus)
		//}

		compilerPath = Paths::engineDir();
#if !PLATFORM_64BITS
#else
		compilerPath.append(TEXT("bin/ThirdParty/Windows/DirectX/x64/d3dcompiler_47.dll"));
#endif

		uint32 compilerFlags = D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY | D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;
		if (DEBUG_SHADER || input.mEnvironment.mCompilerFlags.contains(CFLAG_Debug))
		{
			compilerFlags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
		}
		else
		{
			if (input.mEnvironment.mCompilerFlags.contains(CFLAG_StandardOptimization))
			{
				compilerFlags |= D3D10_SHADER_OPTIMIZATION_LEVEL1;
			}
			else
			{
				compilerFlags |= D3D10_SHADER_OPTIMIZATION_LEVEL3;
			}
		}
		for (int32 flagIndex = 0; flagIndex < input.mEnvironment.mCompilerFlags.size(); flagIndex++)
		{
			compilerFlags |= translateCompilerFlagD3D11((ECompilerFlags)input.mEnvironment.mCompilerFlags[flagIndex]);
		}

		TArray<wstring> filteredErrors;
		if (!compileAndProcessD3DShader(preprocessedShaderSource, compilerPath, compilerFlags, input, entryPointName, shaderProfile, false, filteredErrors, output))
		{
			if (!filteredErrors.size())
			{
				filteredErrors.add(TEXT("compile Failed without errors!"));
			}
		}

		for (int32 errorIndex = 0; errorIndex < filteredErrors.size(); errorIndex++)
		{
			const wstring& currentError = filteredErrors[errorIndex];
			ShaderCompilerError newError;
			int32 firstParenIndex = currentError.find(TEXT("("));
			int32 lastParenIndex = currentError.find(TEXT("):"));
			if (firstParenIndex != INDEX_NONE && lastParenIndex != INDEX_NONE && lastParenIndex > firstParenIndex)
			{
				wstring errorFileAndPath = currentError.substr(0, firstParenIndex);
				wstring extension = Paths::getExtension(errorFileAndPath);
				boost::algorithm::to_upper(extension);

				if (extension == TEXT("USF"))
				{
					newError.mErrorFile = Paths::getCleanFilename(errorFileAndPath);
				}
				else
				{
					newError.mErrorFile = Paths::getCleanFilename(errorFileAndPath) + TEXT(".hlsl");
				}
				newError.mErrorLineString = currentError.substr(firstParenIndex, lastParenIndex - firstParenIndex);
				newError.mStrippedErrorMessage = currentError.substr(lastParenIndex);
			}
			else
			{
				newError.mStrippedErrorMessage = currentError;
			}
			output.mErrors.add(newError);
		}
	}


	void compileShader_windows_sm5(const ShaderCompilerInput& input, struct ShaderCompilerOutput& output, const std::wstring& workingDirectory)
	{
		BOOST_ASSERT(input.mTarget.mPlatform == SP_PCD3D_SM5);
		ShaderCompilerDefinitions additionalDefines;
		additionalDefines.setDefine(TEXT("SM5_PROFILE"), 1);
		compileD3D11Shader(input, output, additionalDefines, workingDirectory);
	}

	void compileShader_windows_sm4(const ShaderCompilerInput& input, struct ShaderCompilerOutput& output, const std::wstring& workingDirectory)
	{
		BOOST_ASSERT(input.mTarget.mPlatform == SP_PCD3D_SM4);
		ShaderCompilerDefinitions additionalDefines;
		additionalDefines.setDefine(TEXT("SM4_PROFILE"), 1);
		compileD3D11Shader(input, output, additionalDefines, workingDirectory);
	}

	void compileShader_windows_es2(const ShaderCompilerInput& input, struct ShaderCompilerOutput& output, const std::wstring& workingDirectory)
	{
		BOOST_ASSERT(input.mTarget.mPlatform == SP_PCD3D_ES2);
		ShaderCompilerDefinitions additionalDefines;
		additionalDefines.setDefine(TEXT("ES2_PROFILE"), 1);
		compileD3D11Shader(input, output, additionalDefines, workingDirectory);
	}
	void compileShader_windows_es3_1(const ShaderCompilerInput& input, struct ShaderCompilerOutput& output, const std::wstring& workingDirectory)
	{
		BOOST_ASSERT(input.mTarget.mPlatform == SP_PCD3D_ES3_1);
		ShaderCompilerDefinitions additionalDefines;
		additionalDefines.setDefine(TEXT("ES3_1_PROFILE"), 1);
		compileD3D11Shader(input, output, additionalDefines, workingDirectory);
	}
}