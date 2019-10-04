SET(CPACK_IFW_VERBOSE ON)

####################
### CONFIGURATION ##
####################

    #generate translations
    file(GLOB_RECURSE IFW_TRANSLATION_FILES "src/ifw/i18n/*.ts")
    qt5_add_translation(IFW_QM_FILES ${IFW_TRANSLATION_FILES})
    add_custom_target(ifw_translations DEPENDS ${IFW_QM_FILES})

    SET(CPACK_GENERATOR IFW)

    SET(APP_DESCRIPTION ${PROJECT_NAME}
        fr "L'experience JdR simplifiée."
    )

    SET(CPACK_IFW_PACKAGE_WIZARD_STYLE "Modern")
    SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_NAME})
    SET(CPACK_IFW_PACKAGE_PUBLISHER ${APP_PUBLISHER})
    SET(CPACK_IFW_PACKAGE_START_MENU_DIRECTORY ${APP_PUBLISHER})
    SET(CPACK_IFW_PRODUCT_URL ${APP_MAIN_URL})
    SET(CPACK_IFW_TARGET_DIRECTORY "@ApplicationsDirX64@/${PROJECT_NAME}")

    SET(APP_REMOTE_SERVER_PATH "//192.168.0.12/www/RPGRPZ")
    SET(APP_ICON_EXT ".ico")

    IF(WIN32)
        SET(CPACK_IFW_ROOT "C:/Qt/Tools/QtInstallerFramework/3.1")
        SET(APP_REMOTE_SERVER_PLATFORM_FOLDER "win")
        SET(APP_INSTALLER_EXTENSION ".exe")
    endif()

    IF(APPLE)
        SET(CPACK_IFW_ROOT "/Qt/Tools/QtInstallerFramework/3.1")
        SET(APP_REMOTE_SERVER_PLATFORM_FOLDER "osx")
        SET(APP_INSTALLER_EXTENSION ".dmg")
    endif()

    #icons
    SET(CPACK_IFW_PACKAGE_LOGO "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/app_64.png")
    SET(CPACK_IFW_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/package${APP_ICON_EXT}")

##############
### INSTALL ##
##############

    INCLUDE(CPack)
    INCLUDE(CPackIFW)

    #find and install redist deps
    IF(WIN32)
        SET(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT "vc_runtime")
        SET(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ".")
        include(InstallRequiredSystemLibraries) #find MSVC_CRT_DIR
        SET(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS "${MSVC_CRT_DIR}/vcruntime140_1.dll") #replace CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS content with dep
        include(InstallRequiredSystemLibraries)
    endif()

    #install bin + libs
    install(
        DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
        DESTINATION .
        COMPONENT ${PROJECT_NAME}
    )

##############
# COMPONENTS #
##############

# vc_runtime #
    IF(WIN32)
        SET(REDIST_COMPONENT_NAME "Visual C++ Redist"
            fr "Redistribuables Visual C++"
        )
        cpack_add_component(vc_runtime)
        cpack_ifw_configure_component(vc_runtime 
            FORCED_INSTALLATION
            VIRTUAL
            DISPLAY_NAME ${REDIST_COMPONENT_NAME}
        )
    endif()

# app #
    cpack_add_component(${PROJECT_NAME}
        DOWNLOADED
    )
    cpack_ifw_configure_component(${PROJECT_NAME}
        FORCED_INSTALLATION
        SCRIPT "src/ifw/install.js"
        USER_INTERFACES "src/ifw/install.ui"
        TRANSLATIONS "${CMAKE_BINARY_DIR}/install_fr.qm"
        DESCRIPTION ${APP_DESCRIPTION}
    )
    cpack_ifw_add_repository(coreRepo URL "${APP_ROOT_URL}/RPGRPZ/packages/${APP_REMOTE_SERVER_PLATFORM_FOLDER}")

###################
## CUSTOM TARGET ## 
###################

#target
SET(APP_REMOTE_SERVER_DOWNLOAD_PATH "${APP_REMOTE_SERVER_PATH}/downloads/${APP_REMOTE_SERVER_PLATFORM_FOLDER}")
SET(APP_REMOTE_SERVER_PACKAGES_PATH "${APP_REMOTE_SERVER_PATH}/packages/${APP_REMOTE_SERVER_PLATFORM_FOLDER}")

#source
SET(APP_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")
SET(APP_PACKAGED_PATH "${CMAKE_BINARY_DIR}/_CPack_Packages/${CPACK_SYSTEM_NAME}/IFW/${APP_PACKAGE_FILE_NAME}")
SET(APP_PACKAGED_INSTALLER_PATH "${APP_PACKAGED_PATH}${APP_INSTALLER_EXTENSION}")
SET(APP_PACKAGED_REPOSITORY_PATH "${APP_PACKAGED_PATH}/repository")

#define custom target
add_custom_target(publishPackage 
    #build
    COMMAND ${CMAKE_COMMAND} --build . --config Release --target package

    #copy to dest
    COMMAND ${CMAKE_COMMAND} -E copy ${APP_PACKAGED_INSTALLER_PATH} ${APP_REMOTE_SERVER_DOWNLOAD_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${APP_PACKAGED_REPOSITORY_PATH} ${APP_REMOTE_SERVER_PACKAGES_PATH}
)
add_dependencies(publishPackage ifw_translations)
