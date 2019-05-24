SET(LIB_NAME TargetPlatform)

SET(SOURCE_FILES 
    ${AIR_DEVELOPER_SOURCE_DIR}/TargetPlatform/TargetPlatformBase.h
    ${AIR_DEVELOPER_SOURCE_DIR}/TargetPlatform/TargetPlatformManagerModule.cpp
    )
    
SET(Interface_FILES
    ${AIR_DEVELOPER_SOURCE_DIR}/TargetPlatform/Interface/IShaderFormat.h
    ${AIR_DEVELOPER_SOURCE_DIR}/TargetPlatform/Interface/ITargetPlatform.h
    ${AIR_DEVELOPER_SOURCE_DIR}/TargetPlatform/Interface/ITargetPlatformManagerModule.h
    ${AIR_DEVELOPER_SOURCE_DIR}/TargetPlatform/Interface/ITargetPlatformModule.h
    ${AIR_DEVELOPER_SOURCE_DIR}/TargetPlatform/Interface/IShaderFormatModule.h
    ${AIR_DEVELOPER_SOURCE_DIR}/TargetPlatform/Interface/ITargetDevice.h
    ${AIR_DEVELOPER_SOURCE_DIR}/TargetPlatform/Interface/TargetDeviceId.h
    )
SOURCE_GROUP(Interface FILES ${Interface_FILES})
    
ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILES}
    ${Interface_FILES}
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/developer)

target_link_libraries(${LIB_NAME}  
    core
    engine
    RenderCore
    RHI
    
)


INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)