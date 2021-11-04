if(AIR_PLATFORM_WINDOWS)
    link_libraries(
        Dwmapi.lib
    )
endif()

SET(CMAKE_DEBUG_POSTFIX "-Debug" CACHE STRING "Add a postfix, usually -Debug on windows")
SET(CMAKE_RELEASE_POSTFIX "" CACHE STRING "Add a postfix, usually empty on windows")
SET(CMAKE_RELWITHDEBINFO_POSTFIX "" CACHE STRING "Add a postfix, usually empty on windows")
SET(CMAKE_MINSIZEREL_POSTFIX "" CACHE STRING "Add a postfix, usually empty on windows")

