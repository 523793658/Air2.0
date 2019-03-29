{
	"targets":[
		{
			"target_name": "launch",
			'variables': {
				'outputSubDir': '',
				"SourceDir": "../source/",
				"RuntimeSourceDir": "<(SourceDir)runtime/",
				"LaunchDir": "<(RuntimeSourceDir)launch/",
				"CoreDir": "<(RuntimeSourceDir)core/",
				"EngineDir": "<(RuntimeSourceDir)engine/",
				"RenderCoreDir": "<(RuntimeSourceDir)RenderCore/",
				"RHIDir": "<(RuntimeSourceDir)RHI/",
				"MovePlayerDir": "<(RuntimeSourceDir)MovePlayer/",
				"SlateDir": "<(RuntimeSourceDir)Slate/",
				"SlateCoreDir": "<(RuntimeSourceDir)SlateCore/",
				"MovieSceneCaptureDir": "<(RuntimeSourceDir)MovieSceneCapture/",
				"RendererDir": "<(RuntimeSourceDir)Renderer/",
				"SlateRHIRendererDir": "<(RuntimeSourceDir)SlateRHIRenderer/",				
				"CoreObjectDir": "<(RuntimeSourceDir)CoreObject/",
				"ShaderCoreDir": "<(RuntimeSourceDir)ShaderCore/",
				"DeveloperSourceDir": "<(SourceDir)developer/",
				"TargetPlatformDir": "<(DeveloperSourceDir)TargetPlatform/",
				"DemoDir": "../source/demo/",
			},
			"type": "executable",
			"include_dirs":[
				"<(CoreDir)",
				"<(LaunchDir)",
				"<(EngineDir)",
				"<(ShaderCoreDir)",
				"<(RenderCoreDir)",
				"<(RHIDir)",
				"<(RendererDir)",
				"<(MovePlayerDir)",
				"<(DemoDir)",
				"<(SlateDir)",
				"<(SlateRHIRendererDir)",
				"<(MovieSceneCaptureDir)",
				"<(CoreObjectDir)",
				"<(SlateCoreDir)",
				"<(TargetPlatformDir)",
				"<(SourceDir)external",
			],
			"dependencies":[
				"core",
				"engine",
				"RenderCore",
				"RHI",
				"D3D11RHI",
				"movePlayer",
				"slateRHIRenderer",
				"Renderer",
				"slate",
				"Demo",
				"ShaderCore",
				"CoreObject",
				"WindowsTargetPlatformModule",
                "EditorEngine",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"sources":[
				"<(LaunchDir)windows/launchWindows.cpp",
				"<(LaunchDir)Launch.cpp",
				"<(LaunchDir)LaunchMininal.h",
				"<(LaunchDir)LaunchEngineLoop.cpp",
				"<(LaunchDir)LaunchEngineLoop.h",
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
									"LAUNCH_RESOURCE",
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