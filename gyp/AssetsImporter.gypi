{
	"targets":[
		{
			"target_name": "AssetsImporter",
			"type": "shared_library",
			'variables': {
                'outputSubDir': '',
				"AssetsImporterDir": "<(EditorDir)AssetsImporter/",
			},
			"include_dirs":[
				
			],
			"sources":[
				"<(AssetsImporterDir)AssetsImporterMain.cpp",
				
			],
			"dependencies":[
				"core",
				"EditorEngine",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
		},
	],
	

	
}