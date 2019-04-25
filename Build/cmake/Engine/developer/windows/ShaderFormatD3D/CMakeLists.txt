SET(LIB_NAME ShaderFormatD3D)

SET(SOURCE_FILES 
    ${AIR_DEVELOPER_SOURCE_DIR}/windows/ShaderFormatD3D/ShaderFormatD3D.cpp
    ${AIR_DEVELOPER_SOURCE_DIR}/windows/ShaderFormatD3D/ShaderFormatD3D.h
    ${AIR_DEVELOPER_SOURCE_DIR}/windows/ShaderFormatD3D/D3D11ShaderCompiler.cpp
    )
    
ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILES}
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/developer/windows)

target_link_libraries(${LIB_NAME}  
    core
    ShaderCore
    ShaderCompilerCommon
    ShaderPreprocessor
    d3dcompiler
    dxguid
)


INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)