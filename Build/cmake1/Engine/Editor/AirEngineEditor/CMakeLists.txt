SET(LIB_NAME EditorEngine)

SET(SOURCE_FILES 
    ${AIR_EDITOR_SOURCE_DIR}/AirEngineEditor/EditorConfig.h
    )
    
SET(SOURCE_CLASSES_FILES 
    ${AIR_EDITOR_SOURCE_DIR}/AirEngineEditor/Classes/Editor/EditorEngine.h
    ${AIR_EDITOR_SOURCE_DIR}/AirEngineEditor/Classes/EditorFramework/AssetImportData.cpp
    ${AIR_EDITOR_SOURCE_DIR}/AirEngineEditor/Classes/EditorFramework/AssetImportData.h
    ${AIR_EDITOR_SOURCE_DIR}/AirEngineEditor/Classes/Factories/Factory.cpp
    ${AIR_EDITOR_SOURCE_DIR}/AirEngineEditor/Classes/Factories/Factory.h    
    )
SOURCE_GROUP(Classes FILES ${SOURCE_CLASSES_FILES})

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILES}
    ${SOURCE_CLASSES_FILES}
)

INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)
target_link_libraries(${LIB_NAME} 
    core
    )

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/Editor)
    
    