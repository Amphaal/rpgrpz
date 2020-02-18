#check required ENV vars
IF(WIN32)
    list(APPEND ENV_VAR_TO_CHECK
        MINGW64_ROOT
    )
    foreach(C_ENV IN LISTS ENV_VAR_TO_CHECK)
        if(NOT DEFINED ENV{${C_ENV}})
            message(FATAL_ERROR "Required ENV variable ${C_ENV} does not exists. Please check README.md for more details !")
        endif()
    endforeach()
endif()