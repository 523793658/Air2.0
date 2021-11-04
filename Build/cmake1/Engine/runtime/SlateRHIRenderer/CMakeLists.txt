SET(LIB_NAME SlateRHIRenderer)

SET(SlateRHIRenderer_SOURCE 
    ${SlateRHIRenderer_SOURCE_DIR}/SlateRHIRenderer.cpp
    ${SlateRHIRenderer_SOURCE_DIR}/SlateRHIRenderer.h
    ${SlateRHIRenderer_SOURCE_DIR}/SlateRHIRendererConfig.h
    ${SlateRHIRenderer_SOURCE_DIR}/SlateRHIRendererModule.cpp
    ${SlateRHIRenderer_SOURCE_DIR}/SlateRHIRenderingPolicy.cpp
    ${SlateRHIRenderer_SOURCE_DIR}/SlateRHIRenderingPolicy.h
    )
    
SET(Interfaces_SOURCE
    ${SlateRHIRenderer_SOURCE_DIR}/Interfaces/ISlate3DRenderer.h
    ${SlateRHIRenderer_SOURCE_DIR}/Interfaces/ISlateRHIRendererModule.h
    )
SOURCE_GROUP(Interfaces FILES ${Interfaces_SOURCE})

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SlateRHIRenderer_SOURCE}
    ${Interfaces_SOURCE}
)
    
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/Runtime)

INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)


target_link_libraries(${LIB_NAME}  
    core
    RenderCore
    RHI
    SlateCore
    )