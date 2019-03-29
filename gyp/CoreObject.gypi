{
	"targets":[
		{
			"target_name": "CoreObject",
			'variables': {
				'outputSubDir': '',
				"SourceDir": "../source/",
				"RuntimeSourceDir": "<(SourceDir)runtime/",
				"EngineSettingDir": "<(RuntimeSourceDir)EngineSetting/",
				"CoreObjectDir": "<(RuntimeSourceDir)CoreObject/",
				"CoreDir": "<(RuntimeSourceDir)core/",
			},
			"type": "shared_library",
			"include_dirs":[
				"<(CoreObjectDir)",
				"<(CoreDir)",
				"<(SourceDir)external",
			],
			"dependencies":[
				"core",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"sources":[
				"<(CoreObjectDir)ObjectGlobals.h",
				"<(CoreObjectDir)Class.h",
				"<(CoreObjectDir)Class.cpp",
				"<(CoreObjectDir)CoreObject.h",
				"<(CoreObjectDir)ObjectGlobals.cpp",
				"<(CoreObjectDir)Object.h",
				"<(CoreObjectDir)ObjectMacros.h",
				"<(CoreObjectDir)Object.cpp",
				"<(CoreObjectDir)ObjectThreadContext.h",
				"<(CoreObjectDir)ObjectThreadContext.cpp",
				"<(CoreObjectDir)ObjectAllocator.h",
				"<(CoreObjectDir)ObjectAllocator.cpp",
				"<(CoreObjectDir)ObjectBase.h",
				"<(CoreObjectDir)ObjectBase.cpp",
				"<(CoreObjectDir)SimpleReflection.h",
				"<(CoreObjectDir)SimpleReflection.cpp",
				"<(CoreObjectDir)LinkerLoad.h",
				"<(CoreObjectDir)LinkerLoad.cpp",
				"<(CoreObjectDir)Linker.h",
				"<(CoreObjectDir)Linker.cpp",
				"<(CoreObjectDir)CoreNative.h",
				"<(CoreObjectDir)CoreNative.cpp",
				"<(CoreObjectDir)/misc/StringClassReference.h",
				"<(CoreObjectDir)/misc/StringAssetsReference.h",
				"<(CoreObjectDir)/Templates/SubclassOf.h",
				"<(CoreObjectDir)/Serialization/bulkData.h",
				"<(CoreObjectDir)/Serialization/bulkData.cpp",
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
									"CORE_OBJECT_SOURCE",
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