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
