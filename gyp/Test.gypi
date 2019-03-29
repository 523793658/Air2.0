{
	"targets":[
		{
			"target_name": "Test",
			"type": "executable",
			'variables': {
				'shaderPath': '../../assets/shader',
				'outputSubDir': '',
				"SourceDir": "../source/",
				"DemoDir": "../source/demo/",
				"TestDir": "../source/Test/",
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
				"<(TestDir)",
			],
			"sources":[
				"<(TestDir)main.cpp",
				
			],
			"dependencies":[
				"core",
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
									"_TEST_",
								],
							},
						},
					},
				],
			],
		},
	],
	

	
}