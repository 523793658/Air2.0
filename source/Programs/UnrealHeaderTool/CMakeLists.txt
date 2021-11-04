add_filtered_std_Iter()


INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/Private")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/Public")
INCLUDE_DIRECTORIES("${AIR_SOURCE_DIR}/launch/Public")


add_executable(${LIB_NAME} 
    ${SOURCE_FILES1}
)

target_link_libraries(${LIB_NAME}
    core
    CoreObject
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER ${FilterPath})

add_compile_definitions("${UperName}_SOURCE")

add_compile_definitions(UBT_MODULE_MANIFEST="UnrealHeaderTool.modules")
add_compile_definitions(UBT_MODULE_MANIFEST_DEBUGGAME="UnrealHeaderTool.modules")
    
INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)
