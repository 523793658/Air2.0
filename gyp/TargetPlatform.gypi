{
	"targets":[
		{
			"target_name": "TargetPlatform",
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
				"<(TargetPlatformDir)Interface/ITargetPlatform.h",
				"<(TargetPlatformDir)Interface/ITargetPlatformModule.h",
				"<(TargetPlatformDir)Interface/ITargetPlatformManagerModule.h",
				"<(TargetPlatformDir)Interface/IShaderFormat.h",
				"<(TargetPlatformDir)Interface/IShaderFormatModule.h",
				"<(TargetPlatformDir)TargetPlatformBase.h",
				"<(TargetPlatformDir)TargetPlatformManagerModule.cpp",
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
									"_TARGET_PLATFORM_",
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