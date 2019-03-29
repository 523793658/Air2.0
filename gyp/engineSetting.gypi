{
	"targets":[
		{
			"target_name": "EngineSetting",
			'variables': {
				'outputSubDir': '',
				"SourceDir": "../source/",
				"RuntimeSourceDir": "<(SourceDir)runtime/",
				"EngineSettingDir": "<(RuntimeSourceDir)EngineSetting/",
				"CoreDir": "<(RuntimeSourceDir)core/",
				"CoreObjectDir": "<(RuntimeSourceDir)CoreObject/",
			},
			"type": "shared_library",
			"include_dirs":[
				"<(EngineSettingDir)",
				"<(CoreDir)",
				"<(SourceDir)external",
				"<(CoreObjectDir)",
			],
			"dependencies":[
				"core",
				"CoreObject",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"sources":[
				"<(EngineSettingDir)Classes/GameMapsSetting.h",
				"<(EngineSettingDir)EngineSetting.h",
				"<(EngineSettingDir)EngineSettingsModule.h",
				"<(EngineSettingDir)EngineSettingsModule.cpp",
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
									"ENGINESETTING_SOURCE",
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