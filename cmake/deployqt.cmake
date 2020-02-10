#please make sure it is registered in PATH !
IF(WIN32)
    SET(DEPLOYQT_EXECUTABLE "windeployqt.exe")
endif()
IF(APPLE)
    SET(DEPLOYQT_EXECUTABLE "macdeployqt")
endif()

# Add commands that copy the Qt runtime to the target's output directory after
# build and install the Qt runtime to the specified directory
function(deployqt target)

    if(WIN32)

        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E
                env "${DEPLOYQT_EXECUTABLE}"
                    --verbose 0
                    --compiler-runtime
                    --no-angle
                    --no-opengl-sw
                    \"$<TARGET_FILE:${target}>/\"
        )
        
    endif()

    if(APPLE)    

        #debug lib ad default as experimental build
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E
                 env "${DEPLOYQT_EXECUTABLE}"
                    \"$<TARGET_FILE:${target}>/../../../\"
                    -verbose=1
                    -use-debug-libs
        )

    endif()

endfunction()

mark_as_advanced(DEPLOYQT_EXECUTABLE)