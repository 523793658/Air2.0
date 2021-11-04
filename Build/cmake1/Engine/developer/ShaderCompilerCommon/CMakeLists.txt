SET(LIB_NAME ShaderCompilerCommon)

SET(SOURCE_FILES 
    ${AIR_DEVELOPER_SOURCE_DIR}/ShaderCompilerCommon/ShaderCompilerCommon.cpp
    ${AIR_DEVELOPER_SOURCE_DIR}/ShaderCompilerCommon/ShaderCompilerCommon.h
    )
    
ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILES}
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/developer)

target_link_libraries(${LIB_NAME}  
    core
)


INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)