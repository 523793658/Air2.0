{
	"targets":[
		{
			"target_name": "RHI",
			'variables': {
				'outputSubDir': '',
				"SourceDir": "../source/",
				"RuntimeSourceDir": "<(SourceDir)runtime/",
				"LaunchDir": "<(RuntimeSourceDir)launch/",
				"CoreDir": "<(RuntimeSourceDir)core/",
				"EngineDir": "<(RuntimeSourceDir)engine/",
				"RHIDir": "<(RuntimeSourceDir)RHI/",
			},
			"type": "shared_library",
			"include_dirs":[
				"<(CoreDir)",
				"<(EngineDir)",
				"<(RHIDir)",
				"<(SourceDir)external",
			],
			"dependencies":[
				"core",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"sources":[
				"<(RHIDir)RHI.h",
				"<(RHIDir)RHI.cpp",
				"<(RHIDir)DynamicRHI.cpp",
				"<(RHIDir)RHIDefinitions.h",
				"<(RHIDir)RHIConfig.h",
				"<(RHIDir)DynamicRHI.h",
				"<(RHIDir)DynamicRHI.cpp",
				"<(RHIDir)RHIResource.h",
				"<(RHIDir)RHIResource.cpp",
				"<(RHIDir)RHICommandList.h",
				"<(RHIDir)RHICommandList.cpp",
				"<(RHIDir)RHICommandList.inl",
				"<(RHIDir)RHICommandListCommandExecutes.inl",
				"<(RHIDir)GPUProfiler.h",
				"<(RHIDir)GPUProfiler.cpp",
				"<(RHIDir)RHIUtilities.h",
				"<(RHIDir)RHIUtilities.cpp",
				"<(RHIDir)RHIStaticStates.h",
				"<(RHIDir)RHIStaticStates.cpp",
				"<(RHIDir)BoundShaderStateCache.h",
				"<(RHIDir)BoundShaderStateCache.cpp",
				
				
				"<(RHIDir)Windows/WindowsDynamicRHI.h",
				"<(RHIDir)Windows/WindowsDynamicRHI.cpp",
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
									"RHI_RESOURCE",
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