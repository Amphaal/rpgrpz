#find helper
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    find_program(WINDEPLOYQT "windeployqt" REQUIRED)
endif()

# Add commands that copy the Qt runtime to the target's output directory after
# build and install the Qt runtime to the specified directory
macro(DeployQt target)
    
    # Run windeployqt immediately after build to determine Qt dependencies
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${WINDEPLOYQT}
                --verbose 0
                --no-angle
                --no-opengl-sw
                --list mapping
                --dry-run
                --translations fr,en
                $<TARGET_FILE:${target}>
                > QtDeps.txt
        BYPRODUCTS "QtDeps.txt"
        COMMENT "List Qt components along executable ${target}"
    )

    # normalize output
    include(PathNormalizer)
    NormalizePath(${target} "QtDeps.txt" "QtDeps_.txt")

    # custom install of those deps
    install(
        SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Qt/DeployQtScript.cmake"
        COMPONENT "Qt"
    )

endmacro()