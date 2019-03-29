{
	"targets":[
		{
			"target_name": "UtilityShader",
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
				"CoreObjectDir": "<(RuntimeSourceDir)CoreObject/",
				"UtilityShaderDir": "<(RuntimeSourceDir)UtilityShader/",
			},
			"type": "shared_library",
			"include_dirs":[
				"<(CoreDir)",
				"<(EngineDir)",
				"<(RHIDir)",
				"<(RenderCoreDir)",
				"<(CoreObjectDir)",
				"<(ShaderCoreDir)",
				"<(SourceDir)external",
			],
			"dependencies":[
				"core",
				"RHI",
				"ShaderCore",
				"engine",
				"RenderCore",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"sources":[
				"<(UtilityShaderDir)OneColorShader.h",
				"<(UtilityShaderDir)OneColorShader.cpp",
				"<(UtilityShaderDir)UtilityShaderConfig.h",
				"<(UtilityShaderDir)ResolveShader.h",
				"<(UtilityShaderDir)ResolveShader.cpp",
				
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
									"UTILITY_SHADER_RESOURCE",
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