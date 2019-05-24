SET(LIB_NAME launch)

SET(SOURCE_FILE
    ${launch_SOURCE_DIR}/windows/launchWindows.cpp 
    ${launch_SOURCE_DIR}/Launch.cpp 
    ${launch_SOURCE_DIR}/LaunchMininal.h 
    ${launch_SOURCE_DIR}/LaunchEngineLoop.cpp 
    ${launch_SOURCE_DIR}/LaunchEngineLoop.h 
)

add_executable(${LIB_NAME} 
    ${SOURCE_FILE}
)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/Runtime)


INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)
target_link_libraries(${LIB_NAME}  
    core
    CoreObject
    Demo
    EditorEngine
    engine
    MovePlayer
    RenderCore
    RHI
    Renderer
    ShaderCore
    Slate
    SlateRHIRenderer
    WindowsTargetPlatform
    )