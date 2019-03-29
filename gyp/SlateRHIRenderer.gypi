{
	"targets":[
		{
			"target_name": "slateRHIRenderer",
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
				"SlateRHIRendererDir": "<(RuntimeSourceDir)SlateRHIRenderer/",
				"RenderCoreDir": "<(RuntimeSourceDir)RenderCore/",
				"MovieSceneCaptureDir": "<(RuntimeSourceDir)MovieSceneCapture/",
				"ShaderCoreDir": "<(RuntimeSourceDir)ShaderCore/",
			},
			"type": "shared_library",
			"include_dirs":[
				"<(CoreDir)",
				"<(EngineDir)",
				"<(SlateDir)",
				"<(SlateRHIRendererDir)",
				"<(RenderCoreDir)",
				"<(RHIDir)",
				"<(ShaderCoreDir)",
				"<(SlateCoreDir)",
				"<(MovieSceneCaptureDir)",
				"<(SourceDir)external",
				
			],
			"dependencies":[
				"core",
				"RHI",
				"slateCore",
				"RenderCore",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"sources":[
				"<(SlateRHIRendererDir)Interfaces/ISlateRHIRendererModule.h",
				"<(SlateRHIRendererDir)Interfaces/ISlate3DRenderer.h",
				"<(SlateRHIRendererDir)SlateRHIRendererConfig.h",
				"<(SlateRHIRendererDir)SlateRHIRenderer.h",
				"<(SlateRHIRendererDir)SlateRHIRenderer.cpp",
				"<(SlateRHIRendererDir)SlateRHIRendererModule.cpp",
				"<(SlateRHIRendererDir)SlateRHIRenderingPolicy.h",
				"<(SlateRHIRendererDir)SlateRHIRenderingPolicy.cpp",
			
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
									"SLATE_RHI_RENDERER_RESOURCE",									
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