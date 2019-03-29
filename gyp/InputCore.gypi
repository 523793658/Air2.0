{
	"targets":[
		{
			"target_name": "InputCore",
			'variables': {
				'outputSubDir': '',
			},
			"type": "shared_library",
			"include_dirs":[
			],
			"dependencies":[
				"core",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"sources":[
				"<(InputCoreDir)InputCoreType.h",
				"<(InputCoreDir)InputCoreMinimal.h",
				"<(InputCoreDir)InputCoreType.cpp",
				"<(InputCoreDir)KeyState.h",
				"<(InputCoreDir)KeyState.cpp",
				
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
									"INPUT_CORE_RESOURCE",
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