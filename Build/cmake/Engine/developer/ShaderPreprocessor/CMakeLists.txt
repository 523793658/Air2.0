SET(LIB_NAME ShaderPreprocessor)

SET(SOURCE_FILES 
    ${AIR_DEVELOPER_SOURCE_DIR}/ShaderPreprocessor/ShaderPreprocessor.cpp
    ${AIR_DEVELOPER_SOURCE_DIR}/ShaderPreprocessor/ShaderPreprocessor.h
    ${AIR_DEVELOPER_SOURCE_DIR}/ShaderPreprocessor/PreprocessorPrivate.h
    )
    
LINK_DIRECTORIES(
    ${AIR_EXTERNAL_SOURCE_DIR}/mcpp-2.7.2/lib/Win64/VS2015
    )
    
ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILES}
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/developer)

target_link_libraries(${LIB_NAME} LINK_PUBLIC 
    core
    engine
    RenderCore
    RHI
    ShaderCore
)
target_include_directories(${LIB_NAME} PUBLIC ${AIR_EXTERNAL_SOURCE_DIR}/dxsdk/Include)

target_link_libraries(${LIB_NAME} LINK_PRIVATE  
    debug mcpp_64d
    optimized mcpp_64
    )

INCLUDE_DIRECTORIES(
    ${AIR_EXTERNAL_SOURCE_DIR}/mcpp-2.7.2/inc
    )
    


INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)