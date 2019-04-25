SET(LIB_NAME ShaderCore)

SET(SOURCE_FILE
     ${ShaderCore_SOURCE_DIR}/ShaderParameters.h  
     ${ShaderCore_SOURCE_DIR}/ShaderParameters.cpp  
     ${ShaderCore_SOURCE_DIR}/ShaderCoreConfig.h  
     ${ShaderCore_SOURCE_DIR}/Shader.h  
     ${ShaderCore_SOURCE_DIR}/Shader.cpp  
     ${ShaderCore_SOURCE_DIR}/ShaderCore.h  
     ${ShaderCore_SOURCE_DIR}/ShaderCore.cpp  
     ${ShaderCore_SOURCE_DIR}/VertexFactory.h  
     ${ShaderCore_SOURCE_DIR}/VertexFactory.cpp  
     ${ShaderCore_SOURCE_DIR}/ShaderCache.h  
     ${ShaderCore_SOURCE_DIR}/ShaderCache.cpp  
     ${ShaderCore_SOURCE_DIR}/ShaderParameterUtils.h  
     ${ShaderCore_SOURCE_DIR}/ShaderParameterUtils.cpp  
)

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILE}
)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER  Engine/Runtime )

target_link_libraries(${LIB_NAME} 
    core
    RHI
    RenderCore
    )
    
INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)