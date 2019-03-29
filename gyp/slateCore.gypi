{
	"targets":[
		{
			"target_name": "slateCore",
			'variables': {
				'outputSubDir': '',
				"SourceDir": "../source/",
				"RuntimeSourceDir": "<(SourceDir)runtime/",
				"LaunchDir": "<(RuntimeSourceDir)launch/",
				"CoreDir": "<(RuntimeSourceDir)core/",
				"EngineDir": "<(RuntimeSourceDir)engine/",
				"RHIDir": "<(RuntimeSourceDir)RHI/",
				"SlateCoreDir": "<(RuntimeSourceDir)SlateCore/",
			},
			"type": "shared_library",
			"include_dirs":[
				"<(CoreDir)",
				"<(EngineDir)",
				"<(RHIDir)",
				"<(SlateCoreDir)",
				"<(SourceDir)external",
			],
			"dependencies":[
				"core",
				"InputCore",
			],
			"includes":[
				"../tools/gyp/common.gypi",
			],
			"sources":[
				"<(SlateCoreDir)Widgets/SWindow.h",
				"<(SlateCoreDir)Widgets/SWindow.cpp",
				"<(SlateCoreDir)Widgets/SCompoundWidget.h",
				"<(SlateCoreDir)Widgets/SWidget.h",
				"<(SlateCoreDir)Widgets/SWidget.cpp",
				"<(SlateCoreDir)Widgets/DeclarativeSyntaxSupport.h",
				"<(SlateCoreDir)Widgets/SUserWidget.h",
				"<(SlateCoreDir)Widgets/SUserWidget.cpp",
				"<(SlateCoreDir)Widgets/SNullWidget.h",
				"<(SlateCoreDir)Widgets/SOverlay.h",
				"<(SlateCoreDir)Widgets/SOverlay.cpp",
				"<(SlateCoreDir)Widgets/SPanel.cpp",
				"<(SlateCoreDir)Widgets/SPanel.h",
				
				"<(SlateCoreDir)Application/SlateApplicationBase.h",
				"<(SlateCoreDir)Application/SlateApplicationBase.cpp",
				"<(SlateCoreDir)Application/SlateWondowHelper.h",
				"<(SlateCoreDir)Application/SlateWondowHelper.cpp",
				

				"<(SlateCoreDir)SlateCore.h",
				
				"<(SlateCoreDir)Input/Events.h",
				"<(SlateCoreDir)Input/Events.cpp",
				"<(SlateCoreDir)Input/Reply.h",
				"<(SlateCoreDir)Input/Reply.cpp",
				"<(SlateCoreDir)Input/ReplyBase.h",
				"<(SlateCoreDir)Input/ReplyBase.cpp",
				
				"<(SlateCoreDir)Rendering/SlateRenderer.h",
				"<(SlateCoreDir)Rendering/SlateRenderer.cpp",
				"<(SlateCoreDir)Rendering/RenderingCommon.h",
				"<(SlateCoreDir)Rendering/RenderingCommon.cpp",
				"<(SlateCoreDir)Rendering/SlateDrawBuffer.h",
				"<(SlateCoreDir)Rendering/SlateDrawBuffer.cpp",
				"<(SlateCoreDir)Rendering/DrawElements.h",
				"<(SlateCoreDir)Rendering/DrawElements.cpp",
				"<(SlateCoreDir)Rendering/ElementBatcher.h",
				"<(SlateCoreDir)Rendering/ElementBatcher.cpp",
				"<(SlateCoreDir)Rendering/RenderingPolicy.h",
				"<(SlateCoreDir)Rendering/SlateLayoutTransform.h",
				"<(SlateCoreDir)Rendering/SlateLayoutTransform.cpp",
				"<(SlateCoreDir)Rendering/SlateRenderTransform.h",
				
				"<(SlateCoreDir)Textures/SlateShaderResource.h",
				"<(SlateCoreDir)Textures/SlateShaderResource.cpp",
				
				"<(SlateCoreDir)Layout/Margin.h",
				"<(SlateCoreDir)Layout/Margin.cpp",
				"<(SlateCoreDir)Layout/WidgetPath.h",
				"<(SlateCoreDir)Layout/WidgetPath.cpp",
				"<(SlateCoreDir)Layout/SlateRect.cpp",
				"<(SlateCoreDir)Layout/SlateRect.h",
				"<(SlateCoreDir)Layout/ArrangedChildren.h",
				"<(SlateCoreDir)Layout/ArrangedChildren.cpp",
				"<(SlateCoreDir)Layout/Geometry.h",
				"<(SlateCoreDir)Layout/Geometry.cpp",
				
				"<(SlateCoreDir)Type/SlateEnums.h",
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
									"SLATE_CORE_RESOURCE",
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