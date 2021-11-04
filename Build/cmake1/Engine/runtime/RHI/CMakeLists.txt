SET(LIB_NAME RHI)

SET(SOURCE_FILE
    ${RHI_SOURCE_DIR}/RHI.h  
    ${RHI_SOURCE_DIR}/RHI.cpp  
    ${RHI_SOURCE_DIR}/DynamicRHI.cpp  
    ${RHI_SOURCE_DIR}/RHIDefinitions.h  
    ${RHI_SOURCE_DIR}/RHIConfig.h  
    ${RHI_SOURCE_DIR}/DynamicRHI.h  
    ${RHI_SOURCE_DIR}/DynamicRHI.cpp  
    ${RHI_SOURCE_DIR}/RHIResource.h  
    ${RHI_SOURCE_DIR}/RHIResource.cpp  
    ${RHI_SOURCE_DIR}/RHIContext.h  
    ${RHI_SOURCE_DIR}/RHICommandList.h  
    ${RHI_SOURCE_DIR}/RHICommandList.cpp  
    ${RHI_SOURCE_DIR}/RHICommandList.inl  
    ${RHI_SOURCE_DIR}/RHICommandListCommandExecutes.inl  
    ${RHI_SOURCE_DIR}/GPUProfiler.h  
    ${RHI_SOURCE_DIR}/GPUProfiler.cpp  
    ${RHI_SOURCE_DIR}/RHIUtilities.h  
    ${RHI_SOURCE_DIR}/RHIUtilities.cpp  
    ${RHI_SOURCE_DIR}/RHIStaticStates.h  
    ${RHI_SOURCE_DIR}/RHIStaticStates.cpp  
    ${RHI_SOURCE_DIR}/BoundShaderStateCache.h  
    ${RHI_SOURCE_DIR}/BoundShaderStateCache.cpp  
    ${RHI_SOURCE_DIR}/RHIShaderFormatDefinitions.inl  
    ${RHI_SOURCE_DIR}/PipelineStateCache.h  
    ${RHI_SOURCE_DIR}/PipelineStateCache.cpp  
    ${RHI_SOURCE_DIR}/PipelineFileCache.h  
    ${RHI_SOURCE_DIR}/PipelineFileCache.cpp  
    )
				
SET(Windows_Source
     ${RHI_SOURCE_DIR}/Windows/WindowsDynamicRHI.h  
     ${RHI_SOURCE_DIR}/Windows/WindowsDynamicRHI.cpp
     )
SOURCE_GROUP(Windows FILES ${Windows_Source})


ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${Windows_Source}
    ${SOURCE_FILE}
)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER  Engine/Runtime )

target_link_libraries(${LIB_NAME} 
    core
    )

INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)
