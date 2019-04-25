SET(LIB_NAME EngineSetting)

SET(SOURCE_FILE
    ${EngineSetting_SOURCE_DIR}/Classes/GameMapsSetting.h
    ${EngineSetting_SOURCE_DIR}/EngineSetting.h
    ${EngineSetting_SOURCE_DIR}/EngineSettingsModule.cpp
    ${EngineSetting_SOURCE_DIR}/EngineSettingsModule.h
)

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILE}
)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER "Engine/Runtime")
target_link_libraries(${LIB_NAME} 
    core
    CoreObject
    )
    
INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)

