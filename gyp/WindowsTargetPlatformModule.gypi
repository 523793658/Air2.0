{
	"targets":[
		{
			"target_name": "WindowsTargetPlatformModule",
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
				"DeveloperWindowsDir": "<(DeveloperSourceDir)windows/",
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
				"<(DeveloperWindowsDir)WindowsTargetPlatform/WindowsTargetPlatformModule.cpp",
				"<(DeveloperWindowsDir)WindowsClientTargetPlatform/WindowsClientTargetPlatformModule.cpp",
				
				
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
									"WINDOWS_TARGET_PLATFORM_RESOURCE",
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