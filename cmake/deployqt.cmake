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

    #define default debug symbols import
    SET(QT_DEPLOY_MUST_BE_DEBUG "")

    # force target
    string(TOLOWER ${CMAKE_BUILD_TYPE} DEPLOYQT_TARGET)
    message("Including QT shared \"${CMAKE_BUILD_TYPE}\" dependencies...")

    if(WIN32)

        #import .pdb
        if(DEPLOYQT_TARGET STREQUAL "debug")
            SET(QT_DEPLOY_MUST_BE_DEBUG "--pdb")
        endif()

        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E
                env "${DEPLOYQT_EXECUTABLE}"
                    --verbose 0
                    --no-compiler-runtime
                    --no-angle
                    --no-opengl-sw
                    ${QT_DEPLOY_MUST_BE_DEBUG}
                    --${DEPLOYQT_TARGET}
                    \"$<TARGET_FILE:${target}>/\"
        )
    endif()

    if(APPLE)

        #use debug libs
        if(DEPLOYQT_TARGET STREQUAL "debug")
            SET(QT_DEPLOY_MUST_BE_DEBUG "-use-debug-libs")
        endif()

        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E
                 env "${DEPLOYQT_EXECUTABLE}"
                    \"$<TARGET_FILE:${target}>/../../../\"
                    -verbose=1
        )

    endif()

endfunction()

mark_as_advanced(DEPLOYQT_EXECUTABLE)