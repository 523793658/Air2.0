{
	"targets":[
		{
			"target_name": "slate",
			'variables': {
				'outputSubDir': '',
				"SourceDir": "../source/",
				"RuntimeSourceDir": "<(SourceDir)runtime/",
				"LaunchDir": "<(RuntimeSourceDir)launch/",
				"CoreDir": "<(RuntimeSourceDir)core/",
				"EngineDir": "<(RuntimeSourceDir)engine/",
				"RHIDir": "<(RuntimeSourceDir)RHI/",
				"SlateCoreDir": "<(RuntimeSourceDir)SlateCore/",
				"SlateDir": "<(RuntimeSourceDir)Slate/",
			},
			"type": "shared_library",
			"include_dirs":[
				"<(CoreDir)",
				"<(EngineDir)",
				"<(SlateDir)",
				"<(RHIDir)",
				"<(SlateCoreDir)",
				"<(SourceDir)external",
			],
			"dependencies":[
				"core",
				"slateCore",
				"InputCore",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"sources":[
				"<(SlateDir)Framework/Application/SlateApplication.h",
				"<(SlateDir)Framework/Application/SlateApplication.cpp",
				"<(SlateDir)Framework/Commands/InputChord.h",
				"<(SlateDir)Framework/Commands/InputChord.cpp",
				"<(SlateDir)Widgets/SViewport.h",
				"<(SlateDir)Widgets/SViewport.cpp",
				"<(SlateDir)Slate.h",
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
									"SLATE_RESOURCE",
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