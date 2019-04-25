SET(LIB_NAME InputCore)

SET(SOURCE_FILE
    ${InputCore_SOURCE_DIR}/InputCoreType.h 
    ${InputCore_SOURCE_DIR}/InputCoreMinimal.h 
    ${InputCore_SOURCE_DIR}/InputCoreType.cpp 
    ${InputCore_SOURCE_DIR}/KeyState.h 
    ${InputCore_SOURCE_DIR}/KeyState.cpp 
)

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILE}
)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/Runtime)

target_link_libraries(${LIB_NAME} 
    core
    )
    
INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)