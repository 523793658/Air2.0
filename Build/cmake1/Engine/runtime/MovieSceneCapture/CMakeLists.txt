SET(LIB_NAME MovieSceneCapture)

SET(SOURCE_FILE
    ${MovieSceneCapture_SOURCE_DIR}/MovieSceneCaptureHandle.h 
    ${MovieSceneCapture_SOURCE_DIR}/MovieSceneCaptureHandle.cpp 
    ${MovieSceneCapture_SOURCE_DIR}/MovieSceneCaptureModule.h 
    ${MovieSceneCapture_SOURCE_DIR}/MovieSceneCaptureModule.cpp 
    ${MovieSceneCapture_SOURCE_DIR}/MovieSceneCaptureConfig.h 
    ${MovieSceneCapture_SOURCE_DIR}/MovieSceneCapture.h 
    ${MovieSceneCapture_SOURCE_DIR}/MovieSceneCapture.cpp 
)

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILE}
)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/Runtime)


INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)