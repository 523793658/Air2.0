set(AIR_CMAKE_MODULE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Modules")
list(APPEND CMAKE_MODULE_PATH "${AIR_CMAKE_MODULE_DIR}")

PROJECT(Air)

CMAKE_MINIMUM_REQUIRED(VERSION 3.9)

set(AIR_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(AIR_CMAKE_MODULE_DIR ${AIR_ROOT_DIR}/Modules)


INCLUDE(${AIR_CMAKE_MODULE_DIR}/Platform.cmake)
INCLUDE(${AIR_CMAKE_MODULE_DIR}/Common.cmake)
INCLUDE(${AIR_CMAKE_MODULE_DIR}/Compiler.cmake)
INCLUDE(${AIR_CMAKE_MODULE_DIR}/CMakeFunc.cmake)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

message(STATUS ${AIR_PROJECT_DIR})

SET(AIR_PROJECT_DIR "${AIR_ROOT_DIR}")

SET(AIR_BIN_DIR "${AIR_ROOT_DIR}/bin/${AIR_PLATFORM_NAME}")
SET(AIR_OUTPUT_DIR "${AIR_ROOT_DIR}/lib/${AIR_PLATFORM_NAME}")
SET(AIR_SOURCE_DIR "${AIR_ROOT_DIR}/source")



ADD_DEFINITIONS(-DUNICODE -D_UNICODE)

add_compile_definitions(WITH_EDITOR=0)
add_compile_definitions(WITH_UNREAL_DEVELOPER_TOOLS=0)
add_compile_definitions(WITH_PLUGIN_SUPPORT=0)
add_compile_definitions(IS_PROGRAM=0)
add_compile_definitions(IS_MONOLITHIC=0)
add_compile_definitions(WITH_SERVER_CODE=1)
add_compile_definitions(__UNREAL__=1)
add_compile_definitions(UE_ENABLE_ICU=1)
add_compile_definitions(UE_APP_NAME=AIR)
add_compile_definitions(USE_STATS_WITHOUT_ENGINE=0)
add_compile_definitions(UE_APP_NAME="AIR")
add_compile_definitions(_GENERATED_BODY)
add_compile_definitions(_GENERATED_BODY_LEGACY)
add_compile_definitions(ENGINE_IS_LICENSEE_VERSION=false)
add_compile_definitions(READ_TARGET_ENABLED_PLUGINS_FROM_RECEIPT=1)
add_compile_definitions(WITH_ACCESSIBILITY=1)
add_compile_definitions(WITH_ICU_V64=1)
add_compile_definitions(HACK_HEADER_GENERATOR)
add_compile_definitions(UBT_MODULE_MANIFEST="Editor.modules")
add_compile_definitions(UBT_MODULE_MANIFEST_DEBUGGAME="Editor-Win64-DebugGame.modules")



add_compile_definitions(BUILD_DEBUG=$<CONFIG:DEBUG>)
add_compile_definitions(OVERRIDE_PLATFORM_HEADER_NAME=${CMAKE_HOST_SYSTEM_NAME})

ADD_SUBDIRECTORY("source")
ADD_SUBDIRECTORY("Extras")
