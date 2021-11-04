SET(LIB_NAME FBXFactory)

SET(SOURCE_FILES 
    ${AIR_EDITOR_SOURCE_DIR}/FBXFactory/FbxFactory.h
    ${AIR_EDITOR_SOURCE_DIR}/FBXFactory/FbxFactory.cpp
    ${AIR_EDITOR_SOURCE_DIR}/FBXFactory/FbxImporter.h
    ${AIR_EDITOR_SOURCE_DIR}/FBXFactory/FbxImporter.cpp
    ${AIR_EDITOR_SOURCE_DIR}/FBXFactory/FbxImportConfig.h
    ${AIR_EDITOR_SOURCE_DIR}/FBXFactory/FbxImportConfig.cpp
    ${AIR_EDITOR_SOURCE_DIR}/FBXFactory/FbxAssetImportData.h
    ${AIR_EDITOR_SOURCE_DIR}/FBXFactory/FbxAssetImportData.cpp
    ${AIR_EDITOR_SOURCE_DIR}/FBXFactory/FbxMeshImportData.h
    ${AIR_EDITOR_SOURCE_DIR}/FBXFactory/FbxMeshImportData.cpp
    ${AIR_EDITOR_SOURCE_DIR}/FBXFactory/FbxStaticMeshImportData.h
    ${AIR_EDITOR_SOURCE_DIR}/FBXFactory/FbxStaticMeshImportData.cpp
    ${AIR_EDITOR_SOURCE_DIR}/FBXFactory/FbxSceneImportData.h
    ${AIR_EDITOR_SOURCE_DIR}/FBXFactory/config.h
    )

link_directories(
    ${AIR_EXTERNAL_SOURCE_DIR}/FBX/2019.2/lib/vs2017/${AIR_ARCH_NAME}
)
    
ADD_LIBRARY(${LIB_NAME}  ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILES}
)



INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)
target_link_libraries(${LIB_NAME} 
    core
    CoreObject
    EditorEngine
    engine
    )
    
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/Editor)

INCLUDE_DIRECTORIES(${AIR_EXTERNAL_SOURCE_DIR}/FBX/2019.2/include)

target_link_libraries(${LIB_NAME}
    libfbxsdk
    )
    
add_compile_definitions(FBXSDK_SHARED)

file(COPY ${AIR_EXTERNAL_SOURCE_DIR}/FBX/2019.2/lib/vs2017/${AIR_ARCH_NAME}/release/libfbxsdk.dll DESTINATION ${AIR_ROOT_DIR}/bin/${AIR_PLATFORM_NAME})
