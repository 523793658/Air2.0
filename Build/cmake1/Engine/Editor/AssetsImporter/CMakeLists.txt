SET(LIB_NAME AssetsImporter)

SET(SOURCE_FILES 
    ${AIR_EDITOR_SOURCE_DIR}/AssetsImporter/AssetsImporterMain.cpp
    )
    
ADD_LIBRARY(${LIB_NAME}  ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILES}
)

INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)
target_link_libraries(${LIB_NAME} 
    core
    EditorEngine
    )

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/Editor)
    