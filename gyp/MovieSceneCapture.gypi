{
	"targets":[
		{
			"target_name": "MovieSceneCapture",
			'variables': {
				'outputSubDir': '',
				"SourceDir": "../source/",
				"RuntimeSourceDir": "<(SourceDir)runtime/",
				"EngineSettingDir": "<(RuntimeSourceDir)EngineSetting/",
				"CoreObjectDir": "<(RuntimeSourceDir)CoreObject/",
				"CoreDir": "<(RuntimeSourceDir)core/",
				"MovieSceneCaptureDir": "<(RuntimeSourceDir)MovieSceneCapture/",
			},
			"type": "shared_library",
			"include_dirs":[
				"<(CoreObjectDir)",
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
				"<(MovieSceneCaptureDir)MovieSceneCaptureHandle.h",
				"<(MovieSceneCaptureDir)MovieSceneCaptureHandle.cpp",
				"<(MovieSceneCaptureDir)MovieSceneCaptureModule.h",
				"<(MovieSceneCaptureDir)MovieSceneCaptureModule.cpp",
				"<(MovieSceneCaptureDir)MovieSceneCaptureConfig.h",
				"<(MovieSceneCaptureDir)MovieSceneCapture.h",
				"<(MovieSceneCaptureDir)MovieSceneCapture.cpp",
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
									"MOVIE_SCENE_CAPTURE_SOURCE",
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