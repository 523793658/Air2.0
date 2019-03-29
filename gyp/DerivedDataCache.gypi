{
	"targets":[
		{
			"target_name": "DerivedDataCache",
			'variables': {
				'outputSubDir': '',
				"SourceDir": "../source/",
				"RuntimeSourceDir": "<(SourceDir)runtime/",
				"DeveloperSourceDir": "<(SourceDir)developer/",
				"LaunchDir": "<(RuntimeSourceDir)launch/",
				"CoreDir": "<(RuntimeSourceDir)core/",
				"EngineDir": "<(RuntimeSourceDir)engine/",
				"RHIDir": "<(RuntimeSourceDir)RHI/",
				"MovePlayerDir": "<(RuntimeSourceDir)MovePlayer/",
				"RenderCoreDir": "<(RuntimeSourceDir)RenderCore/",
				"TargetPlatformDir": "<(DeveloperSourceDir)TargetPlatform/",
				"DerivedDataCacheDir": "<(DeveloperSourceDir)DerivedDataCache/",
			},
			"type": "shared_library",
			"include_dirs":[
				"<(CoreDir)",
				"<(EngineDir)",
				"<(RenderCoreDir)",
				"<(DeveloperSourceDir)",
				"<(RHIDir)",
				"<(MovePlayerDir)",
				"<(SourceDir)external",
				"<(DeveloperSourceDir)TargetPlatform",
				"<(DerivedDataCacheDir)",
			],
			"dependencies":[
				"core",
				"engine",
				"RenderCore",
				"RHI",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"sources":[
				"<(DerivedDataCacheDir)DerivedDataCacheInterface.h",
				"<(DerivedDataCacheDir)DerivedDataCache.cpp",
				"<(DerivedDataCacheDir)DerivedDataBackendInterface.h",
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
									"WINDOWS_DerivedDataCache_RESOURCE",
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