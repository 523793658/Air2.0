{
	"targets":[
		{
			"target_name": "ShaderCompilerCommon",
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
			},
			"type": "shared_library",
			"include_dirs":[
				"<(CoreDir)",
				"<(EngineDir)",
				"<(RHIDir)",
				"<(ShaderCoreDir)",
				"<(RenderCoreDir)",
				"<(SourceDir)external",
			],
			"dependencies":[
				"core",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"sources":[
				"<(ShaderCompilerCommonDir)ShaderCompilerCommon.h",
				"<(ShaderCompilerCommonDir)ShaderCompilerCommon.cpp",
			],
			"conditions":[
				[
					"OS == 'win'",
					{
						"msbuild_configuration_attributes":{
							"CharacterSet": "1",
						}, 
						"msbuild_settings":{
							"ClCompile":{
								'PreprocessorDefinitions': [
									"_SHADER_COMPILER_COMMON_",
								],
							},
							'Link': {
								'AdditionalDependencies':[
									
								],
							},
						},
						"configurations":{
							"Debug":{
								"msbuild_settings":{
									"ClCompile":{
										'RuntimeLibrary': 'MultiThreadedDebugDLL',
									},
									'Link': {
										'GenerateDebugInformation': 'true',
										'AdditionalDependencies':[
										],
									},
								},
							},
							"Release":{
								"msbuild_settings":{
									"ClCompile":{
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