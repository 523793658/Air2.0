{
	"targets":[
		{
			"target_name": "ShaderCore",
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
				"DeveloperSourceDir": "<(SourceDir)developer/",
				"TargetPlatformDir": "<(DeveloperSourceDir)TargetPlatform/",
			},
			"type": "shared_library",
			"include_dirs":[
				"<(CoreDir)",
				"<(EngineDir)",
				"<(RHIDir)",
				"<(ShaderCoreDir)",
				"<(RenderCoreDir)",
				"<(TargetPlatformDir)",
				"<(SourceDir)external",
			],
			"dependencies":[
				"core",
				"RHI",
				"RenderCore",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"sources":[
				"<(ShaderCoreDir)ShaderParameters.h",
				"<(ShaderCoreDir)ShaderParameters.cpp",
				"<(ShaderCoreDir)ShaderCoreConfig.h",
				"<(ShaderCoreDir)Shader.h",
				"<(ShaderCoreDir)Shader.cpp",
				"<(ShaderCoreDir)ShaderCore.h",
				"<(ShaderCoreDir)ShaderCore.cpp",
				"<(ShaderCoreDir)VertexFactory.h",
				"<(ShaderCoreDir)VertexFactory.cpp",
				"<(ShaderCoreDir)ShaderCache.h",
				"<(ShaderCoreDir)ShaderCache.cpp",
				"<(ShaderCoreDir)ShaderParameterUtils.h",
				"<(ShaderCoreDir)ShaderParameterUtils.cpp",
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
									"SHADER_CORE_RESOURCE",
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