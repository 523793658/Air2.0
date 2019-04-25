SET(LIB_NAME DerivedDataCache)

SET(SOURCE_FILES 
    ${AIR_DEVELOPER_SOURCE_DIR}/DerivedDataCache/DerivedDataCache.cpp
    ${AIR_DEVELOPER_SOURCE_DIR}/DerivedDataCache/DerivedDataCacheInterface.h
    ${AIR_DEVELOPER_SOURCE_DIR}/DerivedDataCache/DerivedDataBackendInterface.h
    )
    
ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILES}
)

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/developer)

target_link_libraries(${LIB_NAME}  
    core
    engine
    RenderCore
    RHI
)


INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)