add_filtered_std_Iter()



add_compile_definitions("${UperName}_SOURCE")
    


link_directories(
    ${AIR_EXTERNAL_SOURCE_DIR}/AMD/AMD_AGS/lib/VS2015
    ${AIR_EXTERNAL_SOURCE_DIR}/NVIDIA//nvapi/amd64
    ${AIR_EXTERNAL_SOURCE_DIR}/dxsdk/Lib/${AIR_ARCH_NAME}
)

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILES1}
)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER ${FilterPath})

add_compile_definitions(WITH_D3DX_LIBS)

target_link_libraries(${LIB_NAME} LINK_PUBLIC core RHI engine RenderCore UtilityShader)

target_include_directories(${LIB_NAME} PUBLIC ${AIR_EXTERNAL_SOURCE_DIR}/dxsdk/Include)

target_link_libraries(${LIB_NAME} LINK_PRIVATE
    debug amd_ags_${AIR_ARCH_NAME}d
    optimized amd_ags_${AIR_ARCH_NAME}
    d3d11.lib 
    dxgi.lib
    nvapi64.lib
    dxguid.lib
)
INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)
