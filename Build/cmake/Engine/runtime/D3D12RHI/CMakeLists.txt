SET(LIB_NAME D3D12RHI)

SET(SOURCE_FILE
    ${D3D12RHI_SOURCE_DIR}/D3D12RHI.cpp
    ${D3D12RHI_SOURCE_DIR}/D3D12RHI.h
    
)

link_directories(
    ${AIR_EXTERNAL_SOURCE_DIR}/AMD/AMD_AGS/lib/VS2015
    ${AIR_EXTERNAL_SOURCE_DIR}/NVIDIA/nvapi/amd64
)

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILE}
)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/Runtime)

target_include_directories(${LIB_NAME} PUBLIC ${AIR_EXTERNAL_SOURCE_DIR}/dxsdk/Include)

target_link_libraries(${LIB_NAME} LINK_PUBLIC core RHI engine RenderCore UtilityShader)


target_link_libraries(${LIB_NAME} LINK_PRIVATE
    debug amd_ags_${AIR_ARCH_NAME}d
    optimized amd_ags_${AIR_ARCH_NAME}
    nvapi64.lib
)


INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)
