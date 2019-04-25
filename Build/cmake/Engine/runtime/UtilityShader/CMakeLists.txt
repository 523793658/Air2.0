SET(LIB_NAME UtilityShader)

SET(SOURCE_FILE
    ${UtilityShader_SOURCE_DIR}/OneColorShader.cpp
    ${UtilityShader_SOURCE_DIR}/OneColorShader.h
    ${UtilityShader_SOURCE_DIR}/ResolveShader.cpp
    ${UtilityShader_SOURCE_DIR}/ResolveShader.h
    ${UtilityShader_SOURCE_DIR}/UtilityShaderConfig.h
)

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILE}
)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER "Engine/Runtime")

INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)

target_link_libraries(${LIB_NAME}  
    core 
    engine 
    RenderCore 
    RHI
    ShaderCore
)