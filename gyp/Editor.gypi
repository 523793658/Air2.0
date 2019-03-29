{
	"targets":[
		{
			"target_name": "EditorEngine",
			"type": "shared_library",
            'variables': {
				'outputSubDir': '',
                },
            "sources":[
				"<(AirEditor)Classes/Editor/EditorEngine.h",
				"<(AirEditor)Classes/Factories/Factory.h",
				"<(AirEditor)Classes/Factories/Factory.cpp",
				"<(AirEditor)Classes/EditorFramework/AssetImportData.h",
				"<(AirEditor)Classes/EditorFramework/AssetImportData.cpp",
				"<(AirEditor)EditorConfig.h",
			],
            "dependencies":[
                "core",
            ],
            "includes":[
                "../tools/gyp/common.gypi",
                "./common_include.gypi",
            ],
		},
	],
}