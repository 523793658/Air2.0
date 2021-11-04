add_filtered_std_Iter()
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Public")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Private")
SET(MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/Public" "${CMAKE_CURRENT_SOURCE_DIR}/Private" PARENT_SCOPE)

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILES1}
)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER ${FilterPath})

add_compile_definitions("${UperName}_SOURCE")
    
INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)