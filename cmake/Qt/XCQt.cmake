IF(CMAKE_CROSSCOMPILING)

    # hard-replace a bunch of executable paths defined by Qt's find_package 
    IF(CMAKE_SYSTEM_NAME STREQUAL "Windows")	
        #MOC	
        add_executable(Qt5::moc IMPORTED)	
        find_program(XC_MOC moc REQUIRED)
        message("[${XC_MOC}]")
        set_target_properties(Qt5::moc PROPERTIES	
            IMPORTED_LOCATION ${XC_MOC}	
        )	
        #RCC	
        add_executable(Qt5::rcc IMPORTED)
        find_program(XC_RCC rcc REQUIRED)
        message("[${XC_RCC}]")
        set_target_properties(Qt5::rcc PROPERTIES	
            IMPORTED_LOCATION ${XC_RCC}	
        )	
        #UIC	
        add_executable(Qt5::uic IMPORTED)
        find_program(XC_UIC uic REQUIRED)
        message("[${XC_UIC}]")
        set_target_properties(Qt5::uic PROPERTIES	
            IMPORTED_LOCATION ${XC_UIC}	
        )	
        #lrelease	
        add_executable(Qt5::lrelease IMPORTED)
        find_program(XC_LRELEASE lrelease REQUIRED)
        message("[${XC_LRELEASE}]")
        set_target_properties(Qt5::lrelease PROPERTIES	
            IMPORTED_LOCATION ${XC_LRELEASE}
        )	
        #lupdate	
        add_executable(Qt5::lupdate IMPORTED)
        find_program(XC_LUPDATE lupdate REQUIRED)
        message("[${XC_LUPDATE}]")
        set_target_properties(Qt5::lupdate PROPERTIES	
            IMPORTED_LOCATION ${XC_LUPDATE}	
        )	
    endif()

endif()