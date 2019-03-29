#gtest的相关配置

{
	'include_dirs': [
		'../../external/gtest/include',
	],
	'defines': ['_VARIADIC_MAX=10'],
	'conditions': [
		[ 'OS == "win"',
			{
				'configurations': {
					'Debug': {
						'msbuild_settings': {
							'Link': {
								'AdditionalDependencies': [
									'..\\..\\external\\gtest\\lib\\gtestd.lib',
									'..\\..\\external\\gtest\\lib\\gtest_maind.lib',
								],
							},
						},
					},
					'Release': {
						'msbuild_settings': {
							'Link': {
								'AdditionalDependencies': [
									'..\\..\\external\\gtest\\lib\\gtest.lib',
									'..\\..\\external\\gtest\\lib\\gtest_main.lib',
								],
							},
						},
					},
				},						
			},
		],
	],
}