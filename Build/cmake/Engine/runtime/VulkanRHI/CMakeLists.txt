SET(LIB_NAME VulkanRHI)

SET(WINDOWS_FILE
    ${VulkanRHI_SOURCE_DIR}/Windows/VulkanWindowsPlatform.cpp
    ${VulkanRHI_SOURCE_DIR}/Windows/VulkanWindowsPlatform.h
    ${VulkanRHI_SOURCE_DIR}/Windows/VulkanPlatformDefines.h
)

SOURCE_GROUP(Windows FILES ${WINDOWS_FILE})

SET(SOURCE_FILE
    ${VulkanRHI_SOURCE_DIR}/VulkanDynamicRHI.h
    ${VulkanRHI_SOURCE_DIR}/VulkanRHI.cpp
    ${VulkanRHI_SOURCE_DIR}/VulkanGlobals.h
    ${VulkanRHI_SOURCE_DIR}/VulkanLoader.h
    ${VulkanRHI_SOURCE_DIR}/VulkanGenericPlatform.h
    ${VulkanRHI_SOURCE_DIR}/VulkanGenericPlatform.cpp
    ${VulkanRHI_SOURCE_DIR}/VulkanRHIPrivate.h
    ${VulkanRHI_SOURCE_DIR}/VulkanPlatform.h
    ${VulkanRHI_SOURCE_DIR}/VulkanUtil.cpp
    ${VulkanRHI_SOURCE_DIR}/VulkanUtil.h
    ${VulkanRHI_SOURCE_DIR}/VulkanConfig.h
    ${VulkanRHI_SOURCE_DIR}/VulkanDevice.h
    ${VulkanRHI_SOURCE_DIR}/VulkanDevice.cpp
    ${VulkanRHI_SOURCE_DIR}/VulkanMemory.h
    ${VulkanRHI_SOURCE_DIR}/VulkanMemory.cpp
    ${VulkanRHI_SOURCE_DIR}/VulkanResources.h
    ${VulkanRHI_SOURCE_DIR}/VulkanState.cpp
    ${VulkanRHI_SOURCE_DIR}/VulkanState.h
    ${VulkanRHI_SOURCE_DIR}/VulkanConfiguration.h
    ${VulkanRHI_SOURCE_DIR}/VulkanCommon.h
    ${VulkanRHI_SOURCE_DIR}/VulkanContext.h
    ${VulkanRHI_SOURCE_DIR}/VulkanQueue.cpp
    ${VulkanRHI_SOURCE_DIR}/VulkanQueue.h
    ${VulkanRHI_SOURCE_DIR}/VulkanViewport.h
    ${VulkanRHI_SOURCE_DIR}/VulkanViewport.cpp
    ${VulkanRHI_SOURCE_DIR}/VulkanCommandBuffer.h
    ${VulkanRHI_SOURCE_DIR}/VulkanCommandBuffer.cpp
    ${VulkanRHI_SOURCE_DIR}/VulkanSwapChain.h
    ${VulkanRHI_SOURCE_DIR}/VulkanSwapChain.cpp
    ${VulkanRHI_SOURCE_DIR}/VulkanPipeline.h
    ${VulkanRHI_SOURCE_DIR}/VulkanPipeline.cpp
    ${VulkanRHI_SOURCE_DIR}/VulkanTexture.h
    ${VulkanRHI_SOURCE_DIR}/VulkanTexture.cpp
    ${VulkanRHI_SOURCE_DIR}/VulkanUAV.cpp
    
)

link_directories(
    ${AIR_EXTERNAL_SOURCE_DIR}/AMD/AMD_AGS/lib/VS2015
    ${AIR_EXTERNAL_SOURCE_DIR}/NVIDIA/nvapi/amd64
)

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILE}
    ${WINDOWS_FILE}
)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/Runtime)

target_include_directories(${LIB_NAME} PUBLIC ${AIR_EXTERNAL_SOURCE_DIR}/vulkan/Include)

target_link_libraries(${LIB_NAME} LINK_PUBLIC core RHI engine RenderCore ShaderCore UtilityShader)


target_link_libraries(${LIB_NAME} LINK_PRIVATE
    debug amd_ags_${AIR_ARCH_NAME}d
    optimized amd_ags_${AIR_ARCH_NAME}
    nvapi64.lib
)


INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)
