SET(LIB_NAME MovePlayer)

SET(SOURCE_FILE
    ${MovePlayer_SOURCE_DIR}/MoviePlayerConfig.h 
    ${MovePlayer_SOURCE_DIR}/MoviePlayer.h 
    ${MovePlayer_SOURCE_DIR}/MoviePlayer.cpp 
    ${MovePlayer_SOURCE_DIR}/DefaultGameMoviePlayer.h 
    ${MovePlayer_SOURCE_DIR}/DefaultGameMoviePlayer.cpp 
    ${MovePlayer_SOURCE_DIR}/NullMoviePlayer.h 
    ${MovePlayer_SOURCE_DIR}/NullMoviePlayer.cpp 
)

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILE}
)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER Engine/Runtime)


INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)

target_link_libraries(${LIB_NAME}  
    core
    engine
    RenderCore
    RHI
    
)