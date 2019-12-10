if(NOT DEFINED QT_DEPLOY_BIN_PATH)
    message(FATAL_ERROR "QT_DEPLOY_BIN_PATH have not been set!")
endif()

IF(WIN32)
    SET(QT_DEPLOY_BIN "windeployqt.exe")
endif()
IF(APPLE)
    SET(QT_DEPLOY_BIN "macdeployqt")
endif()

# find the deployqt bin
SET(DEPLOYQT_EXECUTABLE ${QT_DEPLOY_BIN_PATH}/${QT_DEPLOY_BIN})

# Add commands that copy the Qt runtime to the target's output directory after
# build and install the Qt runtime to the specified directory
function(deployqt target)

    if(WIN32)

        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E
                env "${DEPLOYQT_EXECUTABLE}"
                    --verbose 0
                    --no-compiler-runtime
                    --no-angle
                    --no-opengl-sw
                    $<$<CONFIG:DEBUG>:--pdb>
                    $<$<CONFIG:DEBUG>:--debug>
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