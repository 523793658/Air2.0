{
	"targets":[
		{
			"target_name": "Demo",
			"type": "shared_library",
			'variables': {
				'shaderPath': '../../assets/shader',
				'outputSubDir': '',
				"SourceDir": "../source/",
				"DemoDir": "../source/demo/",
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
			},
			"include_dirs":[
				"<(CoreDir)",
				"<(LaunchDir)",
				"<(EngineDir)",
				"<(ShaderCoreDir)",
				"<(RenderCoreDir)",
				"<(RHIDir)",
				"<(RendererDir)",
				"<(MovePlayerDir)",
				"<(TargetPlatformDir)",
				"<(SlateDir)",
				"<(SlateRHIRendererDir)",
				"<(MovieSceneCaptureDir)",
				"<(CoreObjectDir)",
				"<(SlateCoreDir)",
				"<(SourceDir)external",
				"<(DemoDir)",
			],
			"sources":[
				"<(DemoDir)main.cpp",
				"<(DemoDir)DemoType.h",
				"<(DemoDir)ApplicationManager.h",
				"<(DemoDir)ApplicationManager.cpp",
				
				"<(DemoDir)core/DemoViewportClient.h",
				"<(DemoDir)core/DemoViewportClient.cpp",
				"<(DemoDir)core/DemoEngine.h",
				"<(DemoDir)core/DemoEngine.cpp",
				"<(DemoDir)core/DemoConfig.h",
				"<(DemoDir)core/Application.h",
				"<(DemoDir)core/Application.cpp",
				"<(DemoDir)core/DemoInput.h",
				"<(DemoDir)core/DemoInput.cpp",
				"<(DemoDir)core/DemoGameMode.h",
				"<(DemoDir)core/DemoGameMode.cpp",
				
				"<(DemoDir)/Demos/InitEngine/InitEngine.h",
				"<(DemoDir)/Demos/InitEngine/InitEngine.cpp",
			],
			"dependencies":[
				"core",
				"slateCore",
				"slate",
				"RHI",
				"RenderCore",
				"CoreObject",
				"EngineSetting",
				"ShaderCore",
				"MovieSceneCapture",
				"engine",
				"InputCore",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"conditions":[
				[
					"OS == 'win'",
					{
						"msbuild_settings":{
							"ClCompile":{
								'PreprocessorDefinitions': [
									"_DEMO_",
								],
							},
						},
					},
				],
			],
		},
	],
	

	
}