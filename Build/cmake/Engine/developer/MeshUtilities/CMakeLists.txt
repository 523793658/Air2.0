SET(LIB_NAME MeshUtilities)

SET(MeshUtilities_Source
    ${AIR_DEVELOPER_SOURCE_DIR}/MeshUtilities/MeshUtilities.cpp
    ${AIR_DEVELOPER_SOURCE_DIR}/MeshUtilities/MeshUtilities.h
    )

ADD_LIBRARY(${LIB_NAME}  ${AIR_PREFERRED_LIB_TYPE} 
    ${MeshUtilities_Source}
)

target_link_libraries(${LIB_NAME} 
    core
	CoreObject
    engine
    )

INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/developer)