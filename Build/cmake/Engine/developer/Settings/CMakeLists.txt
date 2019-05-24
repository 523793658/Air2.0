SET(LIB_NAME Settings)
SET(SOURCE_FILES 
    ${AIR_DEVELOPER_SOURCE_DIR}/Settings/ISettingsModule.h
    ${AIR_DEVELOPER_SOURCE_DIR}/Settings/SettingsModule.cpp
    ${AIR_DEVELOPER_SOURCE_DIR}/Settings/ISettingsSection.h
    ${AIR_DEVELOPER_SOURCE_DIR}/Settings/SettingsSection.h
    ${AIR_DEVELOPER_SOURCE_DIR}/Settings/SettingsSection.cpp
    ${AIR_DEVELOPER_SOURCE_DIR}/Settings/ISettingsContainer.h
    ${AIR_DEVELOPER_SOURCE_DIR}/Settings/SettingsContainer.h
    ${AIR_DEVELOPER_SOURCE_DIR}/Settings/SettingsContainer.cpp
    ${AIR_DEVELOPER_SOURCE_DIR}/Settings/SettingsCategory.h
    ${AIR_DEVELOPER_SOURCE_DIR}/Settings/SettingsCategory.cpp
    ${AIR_DEVELOPER_SOURCE_DIR}/Settings/ISettingsCategory.h
    )
ADD_LIBRARY(${LIB_NAME}  ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILES}
)
target_link_libraries(${LIB_NAME} 
    core
	CoreObject
    )

INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/developer)