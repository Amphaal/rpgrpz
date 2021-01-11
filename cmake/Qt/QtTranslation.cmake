
macro(HandleQtTranslation target sourceFiles translationFiles)

    #updates TS files from sources
    qt5_create_translation(QM_FILES
        ${sourceFiles}
        ${translationFiles}
    )

    #generate QM files
    qt5_add_translation(APP_BINARY_TRANSLATION_FILES ${translationFiles})

    #generate QM files from TS
    target_sources(${target} PUBLIC
        ${APP_BINARY_TRANSLATION_FILES}
    )

    #copy translations into output
    add_custom_command(TARGET ${target}   
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${target}>/translations       
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${QM_FILES} $<TARGET_FILE_DIR:${target}>/translations
        COMMENT "Copy Qt translation files"
    )

endmacro()
