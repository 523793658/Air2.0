{
	"targets":[
		{
			"target_name": "slateNullRenderer",
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
				"SlateNullRendererDir": "<(RuntimeSourceDir)SlateNullRenderer/",
			},
			"type": "shared_library",
			"include_dirs":[
				"<(CoreDir)",
				"<(EngineDir)",
				"<(SlateDir)",
				"<(SlateNullRendererDir)",
				"<(RHIDir)",
				"<(SlateCoreDir)",
				"<(SourceDir)external",
			],
			"dependencies":[
				"core",
				"slateCore",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"sources":[
				"<(SlateNullRendererDir)Interfaces/ISlateNullRendererModule.h",
				"<(SlateNullRendererDir)SlateNullRenderer.h",
			
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
									"SLATE_NULL_RENDERER_RESOURCE",
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