SET(LIB_NAME DesktopPlatform)
SET(SOURCE_FILES
     ${AIR_DEVELOPER_SOURCE_DIR}/DesktopPlatform/PlatformInfo.h
     ${AIR_DEVELOPER_SOURCE_DIR}/DesktopPlatform/PlatformInfo.cpp
     )
     
ADD_LIBRARY(${LIB_NAME}  ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILES}
)
target_link_libraries(${LIB_NAME} 
    core
    )

INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/developer)