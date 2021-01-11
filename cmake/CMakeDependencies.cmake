###########
# pe-util #
###########

if(MINGW)
    # check if mingw root is set
    if(NOT MINGW64_ROOT)
        message(FATAL_ERROR "MINGW64_ROOT must be set ! Please use a toolchain file !")
    endif()

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
endif()

################
# missing libs #
################

macro(DeployPEDependencies target component pattern)

    # generate requirements
    add_custom_command(TARGET ${target}
        COMMAND 
            ${PELDD_EXEC} -a
            -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} 
            -p ${MINGW64_ROOT}/bin
            ${pattern}
            > PEDeps_${component}.txt
        BYPRODUCTS
            PEDeps_${component}.txt
        COMMENT "Find dependencies"
    )

    # normalize paths
    include(PathNormalizer)
    NormalizePath(${target} "PEDeps_${component}.txt" "PEDeps_${component}_.txt")

    # copy required libs
    add_custom_command(TARGET ${target}
        COMMAND 
            cat PEDeps_${component}_.txt | xargs cp -n -t ${CMAKE_BINARY_DIR}/PEDeps_${component}
        DEPENDS 
            PEDeps_${component}_.txt
        COMMENT "Copy found depedencies along executable"
    )

endmacro()

##
##
##

