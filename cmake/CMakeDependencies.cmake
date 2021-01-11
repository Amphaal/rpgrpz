if(MINGW)

    ###########
    # pe-util #
    ###########

    #pe-util is required to find all .dll dependencies
    include(ExternalProject)
    ExternalProject_Add(peldd
        GIT_REPOSITORY  "https://github.com/Amphaal/pe-util.git"
        INSTALL_COMMAND ""
        UPDATE_DISCONNECTED 1 # prevents from it always being built in search for tag change
    )

    #define exec
    ExternalProject_Get_Property(peldd BINARY_DIR)
    set(PELDD_EXEC "${BINARY_DIR}/peldd")

    ################
    # missing libs #
    ################

    #scan for missing lib dependencies, then fetch and copy them
    if(NOT MINGW64_ROOT)
        message(FATAL_ERROR "MINGW64_ROOT must be set ! Please use a toolchain file !")
    endif()

    #generate requirements
    add_custom_command(TARGET appExec POST_BUILD
        COMMAND 
            ${PELDD_EXEC} -a
            -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} 
            -p ${MINGW64_ROOT}/bin
            $<TARGET_FILE:appExec> 
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gio/* 
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/gst-plugins/*
            | sed -f ${CMAKE_CURRENT_SOURCE_DIR}/cmake/antislash-to-slash.sed
            | tr -d '\\015' 
            > lib_requirements.txt
        DEPENDS 
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/antislash-to-slash.sed
        BYPRODUCTS
            lib_requirements.txt
        COMMENT "Find executable dependencies"
    )

    #copy required libs
    add_custom_command(TARGET appExec POST_BUILD
        COMMAND 
            cat lib_requirements.txt | xargs cp -n -t ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        DEPENDS 
            lib_requirements.txt
        COMMENT "Copy found depedencies along executable"
    )

    ##
    ##
    ##

endif()