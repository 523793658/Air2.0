
SET(SOURCE_FILE
    ${AIR_ROOT_DIR}/Extras/VisualStudioDebugging/UE4.natvis
)
add_custom_target(Visualizers  
    SOURCES ${SOURCE_FILE}
)