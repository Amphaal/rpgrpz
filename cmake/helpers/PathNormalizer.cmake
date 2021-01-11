macro(NormalizePath target inputFile outputFile)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND
            cat ${inputFile}
            | sed -f ${CMAKE_CURRENT_SOURCE_DIR}/cmake/helpers/antislash-to-slash.sed
            | tr -d '\\015'
            | tr -d '""'
            > ${outputFile}
        BYPRODUCTS ${outputFile}
    )
endmacro()