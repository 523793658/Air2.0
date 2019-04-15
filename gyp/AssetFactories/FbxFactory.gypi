{
	"targets":[
		{
			"target_name": "FbxFactory",
			"type": "shared_library",
            'variables': {
                "SourceDir": "../../source/",
				'outputSubDir': 'Factories',
            },
            "sources":[
				"<(FBXFactoryDir)FbxFactory.h",
				"<(FBXFactoryDir)FbxFactory.cpp",
				"<(FBXFactoryDir)FbxImporter.h",
				"<(FBXFactoryDir)FbxImporter.cpp",
				"<(FBXFactoryDir)FbxImportConfig.h",
				"<(FBXFactoryDir)FbxImportConfig.cpp",
                "<(FBXFactoryDir)FbxAssetImportData.h",
				"<(FBXFactoryDir)FbxAssetImportData.cpp",
				"<(FBXFactoryDir)FbxMeshImportData.h",
				"<(FBXFactoryDir)FbxMeshImportData.cpp",
				"<(FBXFactoryDir)FbxStaticMeshImportData.h",
				"<(FBXFactoryDir)FbxStaticMeshImportData.cpp",
				"<(FBXFactoryDir)FbxSceneImportData.h",
				"<(FBXFactoryDir)config.h",
			],
            "include_dirs":[
                "<(ExternalDir)FBX/2019.2/include",
                "<(ExternalDir)FBX/2019.2/include/fbxsdk",
            ],
            "dependencies":[
                "core",
                "EditorEngine",
                "engine",
                "CoreObject",
            ],
            "includes":[
                "../../tools/gyp/common.gypi",
            ],
            "conditions":[
				[
					"OS == 'win'",
					{
                        "msbuild_settings":{
                            "ClCompile":{
                                'PreprocessorDefinitions':[
                                    "FBXSDK_SHARED",
                                ],
                            },
                            'Link': {
                                'AdditionalDependencies':[
                                    "libfbxsdk.lib",
                                ],
                            },
                        },    
						"configurations":{
							"Debug":{
								
							},
							'Debug_x64': {
								'msbuild_settings': {	
									'Link': {
										'conditions':[
                                            [
                                                "MSVS_VERSION == '2015'",
                                                {
                                                    'AdditionalLibraryDirectories':[
                                                        '../source/external/FBX/2019.2/lib/vs2015/x64/debug',
                                                    ],
                                                },
                                            ],
                                            [
                                                "MSVS_VERSION == '2017'",
                                                {
                                                    'AdditionalLibraryDirectories':[
                                                        '../source/external/FBX/2019.2/lib/vs2017/x64/debug',
                                                    ],
                                                },
                                            ],
                                        ],
									},
								},
								'msvs_configuration_platform': 'x64',
							},
							"Release":{
								"msbuild_settings":{
								},
							},
							"Release_x64":{
								"msbuild_settings":{
									'Link': {
										'conditions':[
                                            [
                                                "MSVS_VERSION == '2015'",
                                                {
                                                    'AdditionalLibraryDirectories':[
                                                        '../source/external/FBX/2019.2/lib/vs2015/x64/release',
                                                    ],
                                                },
                                            ],
                                            [
                                                "MSVS_VERSION == '2017'",
                                                {
                                                    'AdditionalLibraryDirectories':[
                                                        '../source/external/FBX/2019.2/lib/vs2017/x64/release',
                                                    ],
                                                },
                                            ],
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