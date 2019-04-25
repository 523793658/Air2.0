SET(LIB_NAME SlateNullRenderer)

SET(SlateNullRenderer_SOURCE 
    ${SlateNullRenderer_SOURCE_DIR}/SlateNullRenderer.h
    ${SlateNullRenderer_SOURCE_DIR}/SlateNullRenderer.cpp
    )
    
SET(Interfaces_SOURCE
    ${SlateNullRenderer_SOURCE_DIR}/Interfaces/ISlateNullRendererModule.h
    )
SOURCE_GROUP(Interfaces FILES ${Interfaces_SOURCE})

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SlateNullRenderer_SOURCE}
    ${Interfaces_SOURCE}
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/Runtime)
    
    
INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)