# 第三方库的配置
#
{
	'conditions' : [
		[ 'OS == "win"',
			{
				'msvs_disabled_warnings': [
					4100,		# 警告：未引用的形参
					4152,		# 警告：表达式中的函数/数据指针转换
					4200,		# 警告：使用了非标准扩展 : 结构/联合中的零大小数组
					4505,		# 警告：未引用的本地函数已移除(局部函数定义了，但未使用)
					4800,		# 警告：将值强制为布尔值“true”或“false”(性能警告)
				], 
				"msvs_postbuild":'copy /Y "$(OutDir)lib\\$(ProjectName)$(TargetExt)" "$(SolutionDir)..\\..\\..\\lib\\$(Platform)\\$(Configuration)"',
				'msbuild_configuration_attributes': {
						'CharacterSet': '1',												# unicode字符集
						'OutputDirectory': '$(SolutionDir)$(Platform)\\$(Configuration)',
						#'IntermediateDirectory': '$(Configuration)\\$(ProjectName)\\obj\\',   #中间目录
				},
				'msbuild_settings': {
						'ClCompile': {
							'TreatWChar_tAsBuiltInType': 'true',  							#将WChar_t视为内置类型
							'ProgramDataBaseFileName': '$(OutDir)pdb\\$(Configuration)\\$(TargetName).pdb', #程序数据库文件名
						},
						'Link': {
							'SubSystem': 'Console',											#控制台
							'ImportLibrary' : '$(IntDir)..\\lib\\$(TargetName).lib', 		#到入库
							'ProgramDatabaseFile': '$(OutDir)pdb\\$(Configuration)\\$(TargetName).pdb', #生成程序数据库文件
							'GenerateDebugInformation': 'false',      						#生成调试信息
							'OutputFile': '$(OutDir)bin\\$(Configuration)\\$(TargetName)$(TargetExt)',	#输出文件
							'ImageHasSafeExceptionHandlers': 'false',						#映像具有安全异常处理程序
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
								'IntrinsicFunctions': 'false'								#启用内部函数（否）
								
							},
							'Link': {
								'GenerateDebugInformation': 'true',
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
								'IntrinsicFunctions': 'false'								#启用内部函数（否）
							},
							'Link': {
								'GenerateDebugInformation': 'true',
								'AdditionalDependencies':[
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
		['OS == "android"',
			{
				'ldflags': 
				[
					'-lz',
					'-llog',
					'-ldl',
					'-lGLESv2',
					'-lEGL',
					'-landroid',
				],
			},
		],
		[ 'OS == "ios" or OS == "iossim"' ,
			{
		  		# 'mac_bundle':1,
				'defines': [
					'IOS',
				],
				'link_settings' : {
					'libraries': [
						'$(SDKROOT)/System/Library/Frameworks/Foundation.framework',
					],
				},
				'xcode_settings' : {
					'CODE_SIGN_IDENTITY': 'iPhone Distribution: Xiamen Woobest Interactive Network Technology Co.,Ltd (245A5Z9XVD)',
					'IPHONEOS_DEPLOYMENT_TARGET': '8.1',
					'DEAD_CODE_STRIPPING': 'NO',
					#'WRAPPER_EXTENSION': 'framework',
					'SYMROOT' : '<(XCODE_BUILD_DIR)/',
					'ARCHS' : '$(ARCHS_STANDARD)',
					'VALID_ARCHS' : 'arm64 armv7 armv7s',
					'SUPPORTED_PLATFORMS' : 'iphoneos iphonesimulator',
					'SDKROOT' : 'iphoneos8.1',
				}, 
			}, 	
		], 
	],    
}
