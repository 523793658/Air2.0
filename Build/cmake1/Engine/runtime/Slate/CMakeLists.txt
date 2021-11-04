SET(LIB_NAME Slate)

SET(SOURCE_FILE
    ${Slate_SOURCE_DIR}/Slate.h
)
SET(Framework_Application_Source
    ${Slate_SOURCE_DIR}/Framework/Application/SlateApplication.cpp
    ${Slate_SOURCE_DIR}/Framework/Application/SlateApplication.h
    )
SOURCE_GROUP(Framework\\Application FILES ${Framework_Application_Source})

SET(Framework_Commands_Source
    ${Slate_SOURCE_DIR}/Framework/Commands/InputChord.cpp
    ${Slate_SOURCE_DIR}/Framework/Commands/InputChord.h
    )
SOURCE_GROUP(Framework\\Commands FILES ${Framework_Commands_Source})

SET(Widgets_Source
    ${Slate_SOURCE_DIR}/Widgets/SViewport.cpp
    ${Slate_SOURCE_DIR}/Widgets/SViewport.h
    )
SOURCE_GROUP(Widgets FILES ${Widgets_Source})

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILE}
    ${Framework_Application_Source}
    ${Framework_Commands_Source}
    ${Widgets_Source}
)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER "Engine/Runtime")

target_link_libraries(${LIB_NAME} 
    core
    SlateCore
    InputCore
    )


INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)