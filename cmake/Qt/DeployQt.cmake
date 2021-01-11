#find helper
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    find_program(WINDEPLOYQT "windeployqt" REQUIRED)
endif()

# Add commands that copy the Qt runtime to the target's output directory after
# build and install the Qt runtime to the specified directory
function(DeployQt target)

    # # Run windeployqt immediately after build
    # add_custom_command(TARGET ${target} 
    #     COMMAND ${WINDEPLOYQT}
    #             --verbose 0
    #             --compiler-runtime
    #             --no-angle
    #             --no-opengl-sw
    #             \"$<TARGET_FILE:${target}>\"
    #     COMMENT "Deploy Qt components along executable ${target}"
    # )

endfunction()