SET(LIB_NAME Demo)

SET(SOURCE_FILE
    ${AIR_DEMO_SOURCE_DIR}/ApplicationManager.h
    ${AIR_DEMO_SOURCE_DIR}/ApplicationManager.cpp
    ${AIR_DEMO_SOURCE_DIR}/DemoType.h
    ${AIR_DEMO_SOURCE_DIR}/main.cpp
    )
    
SET(CORE_FILE
    ${AIR_DEMO_SOURCE_DIR}/core/Application.h
    ${AIR_DEMO_SOURCE_DIR}/core/Application.cpp
    ${AIR_DEMO_SOURCE_DIR}/core/DemoConfig.h
    ${AIR_DEMO_SOURCE_DIR}/core/DemoEngine.h
    ${AIR_DEMO_SOURCE_DIR}/core/DemoEngine.cpp
    ${AIR_DEMO_SOURCE_DIR}/core/DemoGameMode.h
    ${AIR_DEMO_SOURCE_DIR}/core/DemoGameMode.cpp
    ${AIR_DEMO_SOURCE_DIR}/core/DemoInput.h
    ${AIR_DEMO_SOURCE_DIR}/core/DemoInput.cpp
    ${AIR_DEMO_SOURCE_DIR}/core/DemoViewportClient.h
    ${AIR_DEMO_SOURCE_DIR}/core/DemoViewportClient.cpp
    )
SOURCE_GROUP(core FILES ${CORE_FILE})

SET(InitEngine_Files
    ${AIR_DEMO_SOURCE_DIR}/Demos/initEngine/InitEngine.cpp
    ${AIR_DEMO_SOURCE_DIR}/Demos/initEngine/InitEngine.h
    )
SOURCE_GROUP(Demos\\InitEngine FILES ${InitEngine_Files})

ADD_LIBRARY(${LIB_NAME}  ${AIR_PREFERRED_LIB_TYPE} 
    ${CORE_FILE}
    ${SOURCE_FILE}
    ${InitEngine_Files}
)

INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)


target_link_libraries(${LIB_NAME} 
    core
    CoreObject
    EditorEngine
    engine
    EngineSetting
    InputCore
    MovieSceneCapture
    RenderCore
    RHI
    )