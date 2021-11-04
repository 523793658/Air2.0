SET(LIB_NAME CoreObject)

SET(SOURCE_FILE
    ${CoreObject_SOURCE_DIR}/ObjectGlobals.h
    ${CoreObject_SOURCE_DIR}/Class.h
    ${CoreObject_SOURCE_DIR}/Class.cpp
    ${CoreObject_SOURCE_DIR}/CoreObject.h
    ${CoreObject_SOURCE_DIR}/ObjectGlobals.cpp
    ${CoreObject_SOURCE_DIR}/Object.h
    ${CoreObject_SOURCE_DIR}/ObjectMacros.h
    ${CoreObject_SOURCE_DIR}/Object.cpp
    ${CoreObject_SOURCE_DIR}/ObjectThreadContext.h
    ${CoreObject_SOURCE_DIR}/ObjectThreadContext.cpp
    ${CoreObject_SOURCE_DIR}/ObjectAllocator.h
    ${CoreObject_SOURCE_DIR}/ObjectAllocator.cpp
    ${CoreObject_SOURCE_DIR}/ObjectBase.h
    ${CoreObject_SOURCE_DIR}/ObjectBase.cpp
    ${CoreObject_SOURCE_DIR}/SimpleReflection.h
    ${CoreObject_SOURCE_DIR}/SimpleReflection.cpp
    ${CoreObject_SOURCE_DIR}/LinkerLoad.h
    ${CoreObject_SOURCE_DIR}/LinkerLoad.cpp
    ${CoreObject_SOURCE_DIR}/Linker.h
    ${CoreObject_SOURCE_DIR}/Linker.cpp
    ${CoreObject_SOURCE_DIR}/CoreNative.h
    ${CoreObject_SOURCE_DIR}/CoreNative.cpp
    ${CoreObject_SOURCE_DIR}/misc/StringClassReference.h
    ${CoreObject_SOURCE_DIR}/misc/StringAssetsReference.h
    ${CoreObject_SOURCE_DIR}/Templates/SubclassOf.h
    ${CoreObject_SOURCE_DIR}/Serialization/bulkData.h
    ${CoreObject_SOURCE_DIR}/Serialization/bulkData.cpp
)

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILE}
)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/Runtime)

target_link_libraries(${LIB_NAME} 
    core
    )
    
    
INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)

