# 各个平台的设置
#
{		
	'variables': {
		"SourceDir": "../source/",
		"DeveloperSourceDir": "<(SourceDir)developer/",
		"TargetPlatformDir": "<(DeveloperSourceDir)TargetPlatform/",
		"RuntimeSourceDir": "<(SourceDir)runtime/",
		"ExternalDir": "<(SourceDir)external/",
		"LaunchDir": "<(RuntimeSourceDir)launch/",
		"CoreDir": "<(RuntimeSourceDir)core/",
		"EngineDir": "<(RuntimeSourceDir)engine/",
		"RHIDir": "<(RuntimeSourceDir)RHI/",
		"ShaderCoreDir": "<(RuntimeSourceDir)ShaderCore/",
		"RenderCoreDir": "<(RuntimeSourceDir)RenderCore/",
		"DeveloperSourceDir": "<(SourceDir)developer/",
		"TargetPlatformDir": "<(DeveloperSourceDir)TargetPlatform/",
		"RendererDir": "<(RuntimeSourceDir)Renderer/",
		"InputCoreDir": "<(RuntimeSourceDir)InputCore/",
		"SlateCoreDir": "<(RuntimeSourceDir)SlateCore/",
		"CoreObjectDir": "<(RuntimeSourceDir)CoreObject/",
        "EditorDir": "<(SourceDir)Editor/",
        "AirEditor": "<(EditorDir)AirEngineEditor/",
        "FBXFactoryDir" : "<(EditorDir)FBXFactory/",
	},
	"include_dirs":[
		"<(SourceDir)external/boost",
		"<(TargetPlatformDir)",
		"<(ShaderCoreDir)",
		"<(CoreDir)",
		"<(RendererDir)",
		"<(InputCoreDir)",
		"<(ExternalDir)",
		"<(EngineDir)",
		"<(SlateCoreDir)",
		"<(CoreObjectDir)",
		"<(RenderCoreDir)",
		"<(AirEditor)",
		"<(FBXFactoryDir)",
		"<(RHIDir)",
	],
	'conditions' : [
		[ 'OS == "win"',
			{
				'msvs_disabled_warnings': [
					4204,		# 警告：用了非标准扩展 : 非常量聚合初始值设定项
					4100,		# 警告：未引用的形参
					4152,		# 警告：表达式中的函数/数据指针转换
					4200,		# 警告：使用了非标准扩展 : 结构/联合中的零大小数组
					4505,		# 警告：未引用的本地函数已移除(局部函数定义了，但未使用)
					4800,		# 警告：将值强制为布尔值“true”或“false”(性能警告)
					4201,		# 警告：使用了非标准扩展 : 无名称的结构/联合
					4996,		# 警告：scanf的不安全问题
					4819,		# 警告：使用了GB2312字符
					4305,       # 警告：截断从'大类型' 到 '小类型'.
					4306,       # 警告：将'小类型'转换为 '大类型'
				], 
				
				'msbuild_configuration_attributes': {
					'OutputDirectory': '$(SolutionDir)..\\bin\\$(Platform)\\$(Configuration)\\<(outputSubDir)',         		#输出目录
					'IntermediateDirectory': '$(Configuration)\\$(ProjectName)\\obj\\',   #中间目录
					'CharacterSet': '1',
				},
				'product_name': '$(ProjectName)-$(Platform)-$(Configuration)',
				'msbuild_settings': {
					'ClCompile': {
						'TreatWChar_tAsBuiltInType': 'true',  							#将WChar_t视为内置类型
						'WarningLevel': 'Level4',										#警告等级
						'ProgramDataBaseFileName': '$(OutDir)\\$(TargetName).pdb', #程序数据库文件名
						'PreprocessorDefinitions': [
                            "PLATFORM_WINDOWS", 
                            "WITH_EDITOR",
                            "$(ProjectName)_SOURCE",
                            ],      #预处理器定义
						'RuntimeTypeInfo': 'true',      							# /GR-
						'MultiProcessorCompilation': 'true',
						'LanguageStandard': 'stdcpp17',
					},
					'Link': {
						'SubSystem': 'Console',											#控制台
						'ImportLibrary' : '$(IntDir)..\\lib\\$(TargetName).lib', 		#到入库
						#'LinkIncremental': 'true',
						'ProgramDatabaseFile': '$(OutDir)\\$(TargetName).pdb', #生成程序数据库文件
						#'GenerateManifest': 'false',
						'GenerateDebugInformation': 'false',      						#生成调试信息

						'ImageHasSafeExceptionHandlers': 'false',						#映像具有安全异常处理程序
						'AdditionalDependencies': [										#附加依赖项
							'kernel32.lib',
							'gdi32.lib',
							'winspool.lib',
							'comdlg32.lib',
							'advapi32.lib',
							'shell32.lib',
							'ole32.lib',
							'oleaut32.lib',
							'user32.lib',
							'uuid.lib',
							'odbc32.lib',
							'odbccp32.lib',
							'DelayImp.lib',
							'winmm.lib',
							'wsock32.lib',
							"Dwmapi.lib",
						],
					},
				},
				'configurations': {
					'Debug': {
						'msbuild_settings': {
							'ClCompile': {
								'BasicRuntimeChecks': 'EnableFastChecks',					#基本运行时检查（/RTC1）
								'DebugInformationFormat': 'EditAndContinue', 				# editAndContiue (/ZI)
								'Optimization': 'Disabled',           						# optimizeDisabled (/Od)
								'RuntimeLibrary': 'MultiThreadedDebugDLL',
								'IntrinsicFunctions': 'false',								#启用内部函数（否）
								'PreprocessorDefinitions': ["BUILD_DEBUG"],      #预处理器定义
								
							},
							'Link': {
								'GenerateDebugInformation': 'true',
								'AdditionalLibraryDirectories':[
									'../source/external/lib/Win32/Debug',
								],
								'AdditionalDependencies':[
								],
								
							},
						},
						'msbuild_configuration_attributes': {
							'LinkIncremental': 'true',
							'GenerateManifest': 'false',
						},
					},
					'Debug_x64': {
						'msbuild_settings': {	
							'ClCompile': {
								'BasicRuntimeChecks': 'EnableFastChecks',					#基本运行时检查（/RTC1）
								'DebugInformationFormat': 'EditAndContinue', 				# editAndContiue (/ZI)
								'Optimization': 'Disabled',           						# optimizeDisabled (/Od)
								'RuntimeLibrary': 'MultiThreadedDebugDLL',
								'IntrinsicFunctions': 'false',								#启用内部函数（否）
								'PreprocessorDefinitions': ["BUILD_DEBUG"],      #预处理器定义
							},
							'Link': {
								'GenerateDebugInformation': 'true',
								'AdditionalDependencies':[
								],
								'AdditionalLibraryDirectories':[
									'../source/external/lib/x64/Debug',
								],
							},
						},
						'msbuild_configuration_attributes': {
							'LinkIncremental': 'true',
							'GenerateManifest': 'false',
						},
						'msvs_configuration_platform': 'x64',
					},
					'Release': {
						'msbuild_settings': {
							'ClCompile': {
								'DebugInformationFormat': 'ProgramDatabase',      			# programDatabase (/Zi)
								'Optimization': 'MaxSpeed',                					# optimizeDisabled (/O2)
								'WholeProgramOptimization': 'true', 						#/GL
								'RuntimeLibrary': 'MultiThreadedDLL',    						# /GR-
								'IntrinsicFunctions': 'true'								#启用内部函数（是）
							},
							'Link': {
								#'LinkIncremental': 'false',
								#'GenerateManifest': 'true',
								'AdditionalDependencies':[
								],
								'AdditionalLibraryDirectories':[
									'../source/external/lib/Win32/Release',
								],
								'GenerateDebugInformation': 'true',		
							},
						},
						'msbuild_configuration_attributes': {
							'LinkIncremental': 'false',										#不启用增量链接
							'GenerateManifest': 'false',									#不生成manifest
						},
					},
					'Release_x64': {
						'msbuild_settings': {
							'ClCompile': {
								'DebugInformationFormat': 'ProgramDatabase',      			# programDatabase (/Zi)
								'Optimization': 'MaxSpeed',                					# optimizeDisabled (/O2)
								'WholeProgramOptimization': 'true', 						#/GL
								'RuntimeLibrary': 'MultiThreadedDLL',    						# /GR-
								'IntrinsicFunctions': 'true'								#启用内部函数（是）
							},
							'Link': {
								#'LinkIncremental': 'false',
								#'GenerateManifest': 'true',
								'AdditionalDependencies':[
								],
								'AdditionalLibraryDirectories':[
									'../source/external/lib/x64/Debug',
								],
								'GenerateDebugInformation': 'true',		
							},
							
						},
						'msbuild_configuration_attributes': {
							'LinkIncremental': 'false',										#不启用增量链接
							'GenerateManifest': 'false',									#不生成manifest
						},
						'msvs_configuration_platform': 'x64',
					},
				},
			},
		],
	],    
}
