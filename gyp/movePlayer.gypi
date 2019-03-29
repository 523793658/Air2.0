{
	"targets":[
		{
			"target_name": "movePlayer",
			'variables': {
				'outputSubDir': '',
				"SourceDir": "../source/",
				"RuntimeSourceDir": "<(SourceDir)runtime/",
				"LaunchDir": "<(RuntimeSourceDir)launch/",
				"CoreDir": "<(RuntimeSourceDir)core/",
				"EngineDir": "<(RuntimeSourceDir)engine/",
				"RHIDir": "<(RuntimeSourceDir)RHI/",
				"MovePlayerDir": "<(RuntimeSourceDir)MovePlayer/",
				"RenderCoreDir": "<(RuntimeSourceDir)RenderCore/",
				"SlateCoreDir": "<(RuntimeSourceDir)SlateCore/",
				"RendererDir": "<(RuntimeSourceDir)Renderer/",
				"SlateDir": "<(RuntimeSourceDir)Slate/",
				"MovieSceneCaptureDir": "<(RuntimeSourceDir)MovieSceneCapture/",
				"CoreObjectDir": "<(RuntimeSourceDir)CoreObject/",
				"ShaderCoreDir": "<(RuntimeSourceDir)ShaderCore/",
			},
			"type": "shared_library",
			"include_dirs":[
				"<(CoreDir)",
				"<(EngineDir)",
				"<(RenderCoreDir)",
				"<(RHIDir)",
				"<(RendererDir)",
				"<(MovieSceneCaptureDir)",
				"<(SlateCoreDir)",
				"<(SlateDir)",
				"<(ShaderCoreDir)",
				"<(CoreObjectDir)",
				"<(MovePlayerDir)",
				"<(SourceDir)external",
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
				"<(MovePlayerDir)MoviePlayerConfig.h",
				"<(MovePlayerDir)MoviePlayer.h",
				"<(MovePlayerDir)MoviePlayer.cpp",
				"<(MovePlayerDir)DefaultGameMoviePlayer.h",
				"<(MovePlayerDir)DefaultGameMoviePlayer.cpp",
				"<(MovePlayerDir)NullMoviePlayer.h",
				"<(MovePlayerDir)NullMoviePlayer.cpp",
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
									"MOVIE_PLAYER_RESOURCE",
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