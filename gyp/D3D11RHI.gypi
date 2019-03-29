{
	"targets":[
		{
			"target_name": "D3D11RHI",
			'variables': {
				'outputSubDir': '',
				"SourceDir": "../source/",
				"RuntimeSourceDir": "<(SourceDir)runtime/",
				"LaunchDir": "<(RuntimeSourceDir)launch/",
				"DeveloperSourceDir": "<(SourceDir)developer/",
				"CoreDir": "<(RuntimeSourceDir)core/",
				"EngineDir": "<(RuntimeSourceDir)engine/",
				"RHIDir": "<(RuntimeSourceDir)RHI/",
				"SlateCoreDir": "<(RuntimeSourceDir)SlateCore/",
				"SlateDir": "<(RuntimeSourceDir)Slate/",
				"SlateRHIRendererDir": "<(RuntimeSourceDir)SlateRHIRenderer/",
				"RenderCoreDir": "<(RuntimeSourceDir)RenderCore/",
				"D3D11RHIDir": "<(RuntimeSourceDir)D3D11RHI/",
				"HeadMountedDisplayDir": "<(RuntimeSourceDir)HeadMountedDisplay/",
				"MovieSceneCaptureDir": "<(RuntimeSourceDir)MovieSceneCapture/",
				"CoreObjectDir": "<(RuntimeSourceDir)CoreObject/",
				"UtilityShaderDir": "<(RuntimeSourceDir)UtilityShader/",
				"ShaderCoreDir": "<(RuntimeSourceDir)ShaderCore/",
			},
			"type": "shared_library",
			"include_dirs":[
				"<(CoreDir)",
				"<(DeveloperSourceDir)",
				"<(EngineDir)",
				"<(D3D11RHIDir)",
				"<(SlateDir)",
				"<(ShaderCoreDir)",
				"<(SlateRHIRendererDir)",
				"<(RenderCoreDir)",
				"<(MovieSceneCaptureDir)",
				"<(RHIDir)",
				"<(UtilityShaderDir)",
				"<(CoreObjectDir)",
				"<(HeadMountedDisplayDir)",
				"<(SlateCoreDir)",
				"<(SourceDir)external",
				"<(SourceDir)external/dxsdk/include",
			],
			"dependencies":[
				"RHI",
				"core",
				"engine",
				"RenderCore",
				"ShaderCore",
				"UtilityShader",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"sources":[
				"<(D3D11RHIDir)D3D11RHI.h",
				"<(D3D11RHIDir)D3D11RHI.cpp",
				"<(D3D11RHIDir)D3D11DynamicRHI.h",
				"<(D3D11RHIDir)D3D11DynamicRHI.cpp",
				"<(D3D11RHIDir)D3D11Typedefs.h",
				"<(D3D11RHIDir)D3D11Util.h",
				"<(D3D11RHIDir)D3D11Util.cpp",
				"<(D3D11RHIDir)D3D11StateCache.h",
				"<(D3D11RHIDir)D3D11StateCache.cpp",
				"<(D3D11RHIDir)D3D11Texture.h",
				"<(D3D11RHIDir)D3D11Texture.cpp",
				"<(D3D11RHIDir)D3D11Viewport.cpp",
				"<(D3D11RHIDir)D3D11Viewport.h",
				"<(D3D11RHIDir)D3D11Resource.h",
				"<(D3D11RHIDir)D3D11ShaderResource.h",
				"<(D3D11RHIDir)D3D11ShaderResource.cpp",
				"<(D3D11RHIDir)D3D11UnorderedAccessView.h",
				"<(D3D11RHIDir)D3D11UnorderedAccessView.cpp",
				"<(D3D11RHIDir)D3D11Shader.h",
				"<(D3D11RHIDir)D3D11Shader.cpp",
				"<(D3D11RHIDir)D3D11ConstantBuffer.h",
				"<(D3D11RHIDir)D3D11ConstantBuffer.cpp",
				"<(D3D11RHIDir)D3D11State.h",
				"<(D3D11RHIDir)D3D11State.cpp",
				"<(D3D11RHIDir)D3D11Buffer.h",
				"<(D3D11RHIDir)D3D11Buffer.cpp",
				"<(D3D11RHIDir)WindowsD3D11UniformBuffer.h",
				"<(D3D11RHIDir)WindowsD3D11UniformBuffer.cpp",
				"<(D3D11RHIDir)D3D11UniformBuffer.h",
				"<(D3D11RHIDir)D3D11UniformBuffer.cpp",
				"<(D3D11RHIDir)D3D11VertexDeclaration.cpp",
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
									"D3D11RHI_SOURCE",
									"WITH_D3DX_LIBS",
								],
							},
							'Link': {
								'AdditionalDependencies':[
									
								],
								'AdditionalLibraryDirectories':[
									'../source/external/AMD/AMD_AGS/lib/VS2015',
								],
							},
						},
						"configurations":{
							"Debug":{
								"msbuild_settings":{
									'Link': {
										'AdditionalDependencies':[
											"d3d11.lib",
											"dxgi.lib",
											"nvapi.lib",
											"dxguid.lib",
											"amd_ags_x86d.lib",
										],
										'AdditionalLibraryDirectories':[
											'../source/external/dxsdk/Lib/x86',
											'../source/external/NVIDIA/nvapi/x86',
											
										],
									},
								},
							},
							'Debug_x64': {
								'msbuild_settings': {	
									'Link': {
										'AdditionalLibraryDirectories':[
											'../source/external/dxsdk/Lib/x64',
											'../source/external/NVIDIA/nvapi/amd64',
										],
										'AdditionalDependencies':[
											"d3d11.lib",
											"dxgi.lib",
											"nvapi64.lib",
											"dxguid.lib",
											"amd_ags_x64d.lib",
										],
										
									},
								},
								'msvs_configuration_platform': 'x64',
							},
							"Release":{
								"msbuild_settings":{
									'Link': {
										'AdditionalLibraryDirectories':[
											'../source/external/dxsdk/Lib/x86',
											'../source/external/NVIDIA/nvapi/x86',
										],
										'AdditionalDependencies':[
											"d3d11.lib",
											"dxgi.lib",
											"nvapi.lib",
											"dxguid.lib",
											"amd_ags_x86.lib",
										],
									},
								},
							},
							"Release_x64":{
								"msbuild_settings":{
									'Link': {
										'AdditionalLibraryDirectories':[
											'../source/external/dxsdk/Lib/x64',
											'../source/external/NVIDIA/nvapi/amd64',
										],
										'AdditionalDependencies':[
											"d3d11.lib",
											"dxgi.lib",
											"nvapi64.lib",
											"dxguid.lib",
											"amd_ags_x64.lib"
										],
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