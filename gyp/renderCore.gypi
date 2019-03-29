{
	"targets":[
		{
			"target_name": "RenderCore",
			'variables': {
				'outputSubDir': '',
				"SourceDir": "../source/",
				"RuntimeSourceDir": "<(SourceDir)runtime/",
				"LaunchDir": "<(RuntimeSourceDir)launch/",
				"CoreDir": "<(RuntimeSourceDir)core/",
				"EngineDir": "<(RuntimeSourceDir)engine/",
				"RHIDir": "<(RuntimeSourceDir)RHI/",
				"RenderCoreDir": "<(RuntimeSourceDir)RenderCore/",
				"ShaderCoreDir": "<(RuntimeSourceDir)ShaderCore/",
			},
			"type": "shared_library",
			"include_dirs":[
				"<(CoreDir)",
				"<(EngineDir)",
				"<(RHIDir)",
				"<(ShaderCoreDir)",
				"<(SourceDir)external",
			],
			"dependencies":[
				"core",
				"RHI",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"sources":[
				"<(RenderCoreDir)RenderingThread.h",
				"<(RenderCoreDir)RenderingThread.cpp",
				"<(RenderCoreDir)RenderCore.h",
				"<(RenderCoreDir)RenderCore.cpp",
				"<(RenderCoreDir)RenderCommandFence.h",
				"<(RenderCoreDir)RenderCommandFence.cpp",
				"<(RenderCoreDir)TickableObjectRenderThread.h",
				"<(RenderCoreDir)TickableObjectRenderThread.cpp",
				"<(RenderCoreDir)RenderResource.h",
				"<(RenderCoreDir)RenderResource.cpp",
				"<(RenderCoreDir)RenderUtils.h",
				"<(RenderCoreDir)RenderUtils.cpp",
				"<(RenderCoreDir)ConstantBuffer.h",
				"<(RenderCoreDir)ConstantBuffer.cpp",
				"<(RenderCoreDir)PackedNormal.h",
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
									"RENDER_CORE_SOURCE",
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