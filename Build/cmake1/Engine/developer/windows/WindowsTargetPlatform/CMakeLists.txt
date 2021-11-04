SET(LIB_NAME WindowsTargetPlatform)
SET(SOURCE_FILES 
    ${AIR_DEVELOPER_SOURCE_DIR}/Windows/WindowsTargetPlatform/WindowsTargetSettings.h
    ${AIR_DEVELOPER_SOURCE_DIR}/Windows/WindowsTargetPlatform/GenericWindowsTargetPlatform.h
    ${AIR_DEVELOPER_SOURCE_DIR}/Windows/WindowsTargetPlatform/LocalPcTargetDevice.h
    ${AIR_DEVELOPER_SOURCE_DIR}/Windows/WindowsTargetPlatform/LocalPcTargetDevice.cpp
    ${AIR_DEVELOPER_SOURCE_DIR}/Windows/WindowsTargetPlatform/WindowsTargetPlatformClasses.cpp
    ${AIR_DEVELOPER_SOURCE_DIR}/Windows/WindowsTargetPlatform/WindowsTargetPlatformModule.cpp
    )
    
    
ADD_LIBRARY(${LIB_NAME}  ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILES}
)

target_link_libraries(${LIB_NAME} 
    core
	CoreObject
	engine
    )

INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/developer/windows)