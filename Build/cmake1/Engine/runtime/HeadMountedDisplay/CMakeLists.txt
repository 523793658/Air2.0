SET(LIB_NAME HeadMountedDisplay)

SET(SOURCE_FILE
    ${HeadMountedDisplay_SOURCE_DIR}/IHeadMountedDisplayModule.h
    ${HeadMountedDisplay_SOURCE_DIR}/HeadMountedDisplayModule.cpp
)

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILE}
)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER "Engine/Runtime")


INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)