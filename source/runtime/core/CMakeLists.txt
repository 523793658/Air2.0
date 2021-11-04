add_filtered_std_Iter()
SET(MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}/Public" "${CMAKE_CURRENT_SOURCE_DIR}/Private" PARENT_SCOPE)

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/Public/HoloLens")


link_directories(
    ${AIR_EXTERNAL_SOURCE_DIR}/lib/${CMAKE_GENERATOR_PLATFORM}
    ${AIR_EXTERNAL_SOURCE_DIR}/zlib/zlib-1.2.5/lib/Win64
)

ADD_LIBRARY(${LIB_NAME} ${AIR_PREFERRED_LIB_TYPE} 
    ${SOURCE_FILES1}
)



SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES FOLDER ${FilterPath})
add_compile_definitions("${UperName}_SOURCE")
    
INCLUDE(${AIR_CMAKE_MODULE_DIR}/TargetsCommonSetting.cmake)
target_link_libraries(${LIB_NAME} 
    TraceLog
    BuildSettings
    Dbghelp.lib
    )
message("--------CMAKE_DEBUG_POSTFIX----ICU-"${AIR_PLATFORM_NAME}${CMAKE_DEBUG_POSTFIX}.lib)
target_link_libraries(${LIB_NAME}
    optimized
    ICU-${AIR_PLATFORM_NAME}${CMAKE_RELEASE_POSTFIX}.lib
)

target_link_libraries(${LIB_NAME}
    debug
    zlibd_64.lib
    Winmm.lib
    Iphlpapi.lib
    Netapi32.lib
    ICU-${AIR_PLATFORM_NAME}${CMAKE_DEBUG_POSTFIX}.lib
    #mimalloc-${AIR_PLATFORM_NAME}${CMAKE_DEBUG_POSTFIX}.lib
    tbb-${AIR_PLATFORM_NAME}${CMAKE_DEBUG_POSTFIX}.lib
    tbbmalloc-${AIR_PLATFORM_NAME}${CMAKE_DEBUG_POSTFIX}.lib
)
