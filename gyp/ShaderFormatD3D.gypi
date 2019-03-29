{
	"targets":[
		{
			"target_name": "ShaderFormatD3D",
			'variables': {
				'outputSubDir': '',
				"SourceDir": "../source/",
				"RuntimeSourceDir": "<(SourceDir)runtime/",
				"LaunchDir": "<(RuntimeSourceDir)launch/",
				"CoreDir": "<(RuntimeSourceDir)core/",
				"EngineDir": "<(RuntimeSourceDir)engine/",
				"RHIDir": "<(RuntimeSourceDir)RHI/",
				"ShaderCoreDir": "<(RuntimeSourceDir)ShaderCore/",
				"RenderCoreDir": "<(RuntimeSourceDir)RenderCore/",
				"ShaderFormatD3DDir": "<(DeveloperSourceDir)windows/ShaderFormatD3D/",
				"ShaderCompilerCommonDir": "<(DeveloperSourceDir)ShaderCompilerCommon/",
				"ShaderPreprocessorDir": "<(DeveloperSourceDir)ShaderPreprocessor/",
			},
			"type": "shared_library",
			"include_dirs":[
				"<(CoreDir)",
				"<(EngineDir)",
				"<(RHIDir)",
				"<(ShaderCoreDir)",
				"<(RenderCoreDir)",
				"<(ShaderPreprocessorDir)",
				"<(ShaderCompilerCommonDir)",
				"<(SourceDir)external",
				"<(SourceDir)external/dxsdk/include",
			],
			"dependencies":[
				"core",
				"ShaderCore",
				"ShaderCompilerCommon",
				"ShaderPreprocessor",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"sources":[
				"<(ShaderFormatD3DDir)ShaderFormatD3D.h",
				"<(ShaderFormatD3DDir)ShaderFormatD3D.cpp",
				"<(ShaderFormatD3DDir)D3D11ShaderCompiler.cpp",
			],
			"conditions":[
				[
					"OS == 'win'",
					{
						"configurations":{
							"Debug":{
								"msbuild_settings":{
									'Link': {
										'AdditionalDependencies':[
											"d3dcompiler.lib",
											"d3d11.lib",
											"dxguid.lib",
										],
										'AdditionalLibraryDirectories':[
											'../source/external/dxsdk/Lib/x86',
											
										],
									},
								},
							},
							'Debug_x64': {
								'msbuild_settings': {	
									'Link': {
										'AdditionalLibraryDirectories':[
											'../source/external/dxsdk/Lib/x64',
										],
										'AdditionalDependencies':[
											"d3dcompiler.lib",
											"d3d11.lib",
											"dxguid.lib",
										],
										
									},
								},
								'msvs_configuration_platform': 'x64',
							},
							"Release":{
								"msbuild_settings":{
									'Link': {
										'AdditionalLibraryDirectories':[
											'../source/external/dxsdk/Lib/x86',
										],
										'AdditionalDependencies':[
											"d3dcompiler.lib",
											"d3d11.lib",
											"dxguid.lib",
										],
									},
								},
							},
							"Release_x64":{
								"msbuild_settings":{
									'Link': {
										'AdditionalLibraryDirectories':[
											'../source/external/dxsdk/Lib/x64',
										],
										'AdditionalDependencies':[
											"d3dcompiler.lib",
											"d3d11.lib",
											"dxguid.lib",
										],
									},
								},
							},
						},
					},
				],
			],
		},
	],
}