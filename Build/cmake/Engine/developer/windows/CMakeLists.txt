ADD_SUBDIRECTORY("ShaderFormatD3D")


SET(LIB_NAME WindowsTargetPlatformModule)

SET(SOURCE_FILES 
    ${AIR_DEVELOPER_SOURCE_DIR}/windows/WindowsTargetPlatform/WindowsTargetPlatformModule.cpp
    ${AIR_DEVELOPER_SOURCE_DIR}/windows/WindowsClientTargetPlatform/WindowsClientTargetPlatformModule.cpp
    )
    
ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILES}
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/developer/windows)

target_link_libraries(${LIB_NAME}  
    core
)


INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)