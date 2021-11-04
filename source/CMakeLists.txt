SET(AIR_RUNTIME_SOURCE_DIR "${AIR_SOURCE_DIR}/runtime")
SET(AIR_DEMO_SOURCE_DIR "${AIR_SOURCE_DIR}/demo")
SET(AIR_DEVELOPER_SOURCE_DIR "${AIR_SOURCE_DIR}/developer")
SET(AIR_EDITOR_SOURCE_DIR "${AIR_SOURCE_DIR}/editor")
SET(AIR_TEST_SOURCE_DIR "${AIR_SOURCE_DIR}/Test")
SET(AIR_EXTERNAL_SOURCE_DIR "${AIR_SOURCE_DIR}/external")

file(GLOB TMP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/runtime/*")
SET(INCLUDE_DIRS "")
foreach(dir ${TMP_FILES})
	if(IS_DIRECTORY ${dir})
		file(GLOB ProjectDir "${dir}/*")
		foreach(subDir ${ProjectDir})
			if(IS_DIRECTORY ${subDir})
				SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${subDir})
			endif()
		endforeach()
		SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${dir})
	endif()
endforeach()

file(GLOB TMP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/developer/*")
foreach(dir ${TMP_FILES})
	if(IS_DIRECTORY ${dir})
		file(GLOB ProjectDir "${dir}/*")
		foreach(subDir ${ProjectDir})
			if(IS_DIRECTORY ${subDir})
				SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${subDir})
			endif()
		endforeach()
		SET(INCLUDE_DIRS ${INCLUDE_DIRS} ${dir})
	endif()
endforeach()		


INCLUDE_DIRECTORIES(
    "${AIR_EXTERNAL_SOURCE_DIR}"
    "${AIR_EXTERNAL_SOURCE_DIR}/boost"
    "${AIR_EXTERNAL_SOURCE_DIR}/Intel/TBB/include"
    "${AIR_EXTERNAL_SOURCE_DIR}/Intel/VTune/VTune-2019/include"
    "${AIR_EXTERNAL_SOURCE_DIR}/mimalloc/include"
    "${AIR_EXTERNAL_SOURCE_DIR}/ICU/source/common"
    "${AIR_EXTERNAL_SOURCE_DIR}/ICU/source/i18n"
    "${AIR_DEVELOPER_SOURCE_DIR}/DerivedDataCache"
    "${AIR_DEVELOPER_SOURCE_DIR}/ShaderCompilerCommon"
    "${AIR_DEVELOPER_SOURCE_DIR}/ShaderPreprocessor"
    "${AIR_DEVELOPER_SOURCE_DIR}/TargetPlatform"
    "${AIR_DEVELOPER_SOURCE_DIR}"
    "${AIR_DEVELOPER_SOURCE_DIR}/windows"
    "${AIR_DEMO_SOURCE_DIR}"
    "${AIR_EDITOR_SOURCE_DIR}/AirEngineEditor"
    "${AIR_EDITOR_SOURCE_DIR}/AssetsImporter"
    "${AIR_EDITOR_SOURCE_DIR}/FBXFactory"
    "${INCLUDE_DIRS}")
	
	
link_directories(
	${AIR_EXTERNAL_SOURCE_DIR}/zlib/zlib-1.2.5/lib/Win64
	${AIR_EXTERNAL_SOURCE_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}
)
	
ADD_SUBDIRECTORY("demo")
ADD_SUBDIRECTORY("developer")
ADD_SUBDIRECTORY("Editor")
ADD_SUBDIRECTORY("runtime")
ADD_SUBDIRECTORY("Test")
ADD_SUBDIRECTORY("Shaders")
ADD_SUBDIRECTORY("external")
ADD_SUBDIRECTORY("Programs")
foreach(item ${MODULE_INCLUDE_DIRS})
	message(${item})
	INCLUDE_DIRECTORIES("${item}")
endforeach()




