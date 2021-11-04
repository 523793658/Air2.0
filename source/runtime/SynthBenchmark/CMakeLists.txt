add_filtered_std_Iter()

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILES1}
)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER ${FilterPath})

add_compile_definitions("${UperName}_SOURCE")
    
target_link_libraries(${LIB_NAME}
    CORE
)

INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)
