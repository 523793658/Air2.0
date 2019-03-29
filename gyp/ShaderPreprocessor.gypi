{
	"targets":[
		{
			"target_name": "ShaderPreprocessor",
			'variables': {
				'outputSubDir': '',
				"SourceDir": "../source/",
				"RuntimeSourceDir": "<(SourceDir)runtime/",
				"DeveloperSourceDir": "<(SourceDir)developer/",
				"LaunchDir": "<(RuntimeSourceDir)launch/",
				"CoreDir": "<(RuntimeSourceDir)core/",
				"EngineDir": "<(RuntimeSourceDir)engine/",
				"RHIDir": "<(RuntimeSourceDir)RHI/",
				"MovePlayerDir": "<(RuntimeSourceDir)MovePlayer/",
				"RenderCoreDir": "<(RuntimeSourceDir)RenderCore/",
				"ShaderPreprocessorDir": "<(DeveloperSourceDir)ShaderPreprocessor/",
			},
			"type": "shared_library",
			"include_dirs":[
				"<(CoreDir)",
				"<(EngineDir)",
				"<(RenderCoreDir)",
				"<(DeveloperSourceDir)",
				"<(RHIDir)",
				"<(MovePlayerDir)",
				"<(SourceDir)external",
				"<(ShaderPreprocessorDir)",
				"<(ExternalDir)/mcpp-2.7.2/inc",
			],
			"dependencies":[
				"core",
				"engine",
				"RenderCore",
				"ShaderCore",
				"RHI",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"sources":[
				"<(ShaderPreprocessorDir)PreprocessorPrivate.h",
				"<(ShaderPreprocessorDir)ShaderPreprocessor.h",
				"<(ShaderPreprocessorDir)ShaderPreprocessor.cpp",
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
									"_SHADER_PREPROCESSOR_",
								],
							},
							'Link': {
								'AdditionalDependencies':[
									
								],
								'AdditionalLibraryDirectories':[
									'../source/external/mcpp-2.7.2/lib/Win64/VS2015',
								],
							},
						},
						"configurations":{
							"Debug":{
								"msbuild_settings":{
									'Link': {
										'AdditionalDependencies':[
											"mcpp_64d.lib",
										],
										'AdditionalLibraryDirectories':[
											'../source/external/mcpp-2.7.2/lib/Win64/x86',
											
										],
									},
								},
							},
							'Debug_x64': {
								'msbuild_settings': {	
									'Link': {
										'AdditionalLibraryDirectories':[
											'../source/external/mcpp-2.7.2/lib/Win64/x64',
										],
										'AdditionalDependencies':[
											"mcpp_64d.lib",
										],
										
									},
								},
								'msvs_configuration_platform': 'x64',
							},
							"Release":{
								"msbuild_settings":{
									'Link': {
										'AdditionalLibraryDirectories':[
											'../source/external/mcpp-2.7.2/lib/Win64/x86',
										],
										'AdditionalDependencies':[
											"mcpp_64.lib",
										],
									},
								},
							},
							"Release_x64":{
								"msbuild_settings":{
									'Link': {
										'AdditionalLibraryDirectories':[
											'../source/external/mcpp-2.7.2/lib/Win64/x64',
										],
										'AdditionalDependencies':[
											"mcpp_64.lib",
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