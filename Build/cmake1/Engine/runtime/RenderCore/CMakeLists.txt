SET(LIB_NAME RenderCore)

SET(SOURCE_FILE
    ${RenderCore_SOURCE_DIR}/RenderingThread.h 
    ${RenderCore_SOURCE_DIR}/RenderingThread.cpp 
    ${RenderCore_SOURCE_DIR}/RenderCore.h 
    ${RenderCore_SOURCE_DIR}/RenderCore.cpp 
    ${RenderCore_SOURCE_DIR}/RenderCommandFence.h 
    ${RenderCore_SOURCE_DIR}/RenderCommandFence.cpp 
    ${RenderCore_SOURCE_DIR}/TickableObjectRenderThread.h 
    ${RenderCore_SOURCE_DIR}/TickableObjectRenderThread.cpp 
    ${RenderCore_SOURCE_DIR}/RenderResource.h 
    ${RenderCore_SOURCE_DIR}/RenderResource.cpp 
    ${RenderCore_SOURCE_DIR}/RenderUtils.h 
    ${RenderCore_SOURCE_DIR}/RenderUtils.cpp 
    ${RenderCore_SOURCE_DIR}/ConstantBuffer.h 
    ${RenderCore_SOURCE_DIR}/ConstantBuffer.cpp 
    ${RenderCore_SOURCE_DIR}/PackedNormal.h
    ${RenderCore_SOURCE_DIR}/ShaderParameterMacros.cpp
    ${RenderCore_SOURCE_DIR}/ShaderParameterMacros.h
    ${RenderCore_SOURCE_DIR}/ShaderParameterMetadata.h
    ${RenderCore_SOURCE_DIR}/ShaderParameterMetadata.cpp
    ${RenderCore_SOURCE_DIR}/ShaderParameterStruct.cpp
    ${RenderCore_SOURCE_DIR}/ShaderParameterMap.h
    ${RenderCore_SOURCE_DIR}/ShaderParameterMap.cpp
    ${RenderCore_SOURCE_DIR}/Shader.h
    ${RenderCore_SOURCE_DIR}/Shader.cpp
    ${RenderCore_SOURCE_DIR}/VertexFactory.h
    ${RenderCore_SOURCE_DIR}/VertexFactory.cpp
    ${RenderCore_SOURCE_DIR}/ShaderParameters.h
    ${RenderCore_SOURCE_DIR}/ShaderParameters.cpp
    ${RenderCore_SOURCE_DIR}/VirtualTexturing.h
    ${RenderCore_SOURCE_DIR}/VirtualTexturing.cpp
    ${RenderCore_SOURCE_DIR}/ShaderPermutation.h
    ${RenderCore_SOURCE_DIR}/DynamicBufferAllocator.h
    ${RenderCore_SOURCE_DIR}/DynamicBufferAllocator.cpp
	${RenderCore_SOURCE_DIR}/ShaderCoreConfig.h  
    ${RenderCore_SOURCE_DIR}/ShaderCore.h  
    ${RenderCore_SOURCE_DIR}/ShaderCore.cpp  
    ${RenderCore_SOURCE_DIR}/ShaderCache.h  
    ${RenderCore_SOURCE_DIR}/ShaderCache.cpp  
    ${RenderCore_SOURCE_DIR}/ShaderParameterUtils.h  
    ${RenderCore_SOURCE_DIR}/ShaderParameterUtils.cpp  
    ${RenderCore_SOURCE_DIR}/CrossCompilerCommon.h  
)

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILE}
)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/Runtime)


target_link_libraries(${LIB_NAME} 
    core
    RHI
    )
    
INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)
