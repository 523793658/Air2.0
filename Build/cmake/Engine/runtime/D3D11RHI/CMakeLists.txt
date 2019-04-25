SET(LIB_NAME D3D11RHI)

SET(SOURCE_FILE
    ${D3D11RHI_SOURCE_DIR}/D3D11RHI.h
    ${D3D11RHI_SOURCE_DIR}/D3D11RHI.cpp
    ${D3D11RHI_SOURCE_DIR}/D3D11DynamicRHI.h
    ${D3D11RHI_SOURCE_DIR}/D3D11DynamicRHI.cpp
    ${D3D11RHI_SOURCE_DIR}/D3D11Typedefs.h
    ${D3D11RHI_SOURCE_DIR}/D3D11Util.h
    ${D3D11RHI_SOURCE_DIR}/D3D11Util.cpp
    ${D3D11RHI_SOURCE_DIR}/D3D11StateCache.h
    ${D3D11RHI_SOURCE_DIR}/D3D11StateCache.cpp
    ${D3D11RHI_SOURCE_DIR}/D3D11Texture.h
    ${D3D11RHI_SOURCE_DIR}/D3D11Texture.cpp
    ${D3D11RHI_SOURCE_DIR}/D3D11Viewport.cpp
    ${D3D11RHI_SOURCE_DIR}/D3D11Viewport.h
    ${D3D11RHI_SOURCE_DIR}/D3D11Resource.h
    ${D3D11RHI_SOURCE_DIR}/D3D11ShaderResource.h
    ${D3D11RHI_SOURCE_DIR}/D3D11ShaderResource.cpp
    ${D3D11RHI_SOURCE_DIR}/D3D11UnorderedAccessView.h
    ${D3D11RHI_SOURCE_DIR}/D3D11UnorderedAccessView.cpp
    ${D3D11RHI_SOURCE_DIR}/D3D11Shader.h
    ${D3D11RHI_SOURCE_DIR}/D3D11Shader.cpp
    ${D3D11RHI_SOURCE_DIR}/D3D11ConstantBuffer.h
    ${D3D11RHI_SOURCE_DIR}/D3D11ConstantBuffer.cpp
    ${D3D11RHI_SOURCE_DIR}/D3D11State.h
    ${D3D11RHI_SOURCE_DIR}/D3D11State.cpp
    ${D3D11RHI_SOURCE_DIR}/D3D11Buffer.h
    ${D3D11RHI_SOURCE_DIR}/D3D11Buffer.cpp
    ${D3D11RHI_SOURCE_DIR}/WindowsD3D11UniformBuffer.h
    ${D3D11RHI_SOURCE_DIR}/WindowsD3D11UniformBuffer.cpp
    ${D3D11RHI_SOURCE_DIR}/D3D11UniformBuffer.h
    ${D3D11RHI_SOURCE_DIR}/D3D11UniformBuffer.cpp
    ${D3D11RHI_SOURCE_DIR}/D3D11VertexDeclaration.cpp
)

link_directories(
    ${AIR_EXTERNAL_SOURCE_DIR}/AMD/AMD_AGS/lib/VS2015
    ${AIR_EXTERNAL_SOURCE_DIR}/NVIDIA//nvapi/amd64
    ${AIR_EXTERNAL_SOURCE_DIR}/dxsdk/Lib/${AIR_ARCH_NAME}
)

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILE}
)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/Runtime)

add_compile_definitions(WITH_D3DX_LIBS)

target_link_libraries(${LIB_NAME} LINK_PUBLIC core RHI engine RenderCore ShaderCore UtilityShader)

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
