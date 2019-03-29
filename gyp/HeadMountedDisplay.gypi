{
	"targets":[
		{
			"target_name": "HeadMountedDisplay",
			'variables': {
				'outputSubDir': '',
				"SourceDir": "../source/",
				"RuntimeSourceDir": "<(SourceDir)runtime/",
				"LaunchDir": "<(RuntimeSourceDir)launch/",
				"CoreDir": "<(RuntimeSourceDir)core/",
				"HeadMountedDisplayDir": "<(RuntimeSourceDir)HeadMountedDisplay/",
			},
			"type": "shared_library",
			"include_dirs":[
				"<(CoreDir)",
				"<(SourceDir)external",
			],
			"dependencies":[
				"core",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"sources":[
				"<(HeadMountedDisplayDir)IHeadMountedDisplayModule.h",
				"<(HeadMountedDisplayDir)HeadMountedDisplayModule.cpp",
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
									"HEADMOUNTEDDISPLAY_SOURCE",
								],
							},
							'Link': {
								'AdditionalDependencies':[
									
								],
							},
						},
					},
				],
			],
		},
	],
}