##########################
### CPACK CONFIGURATION ##
##########################

    SET(CPACK_GENERATOR IFW)

    SET(APP_DESCRIPTION ${PROJECT_NAME}
        fr "L'experience JdR simplifiée."
    )

    SET(CPACK_IFW_PACKAGE_WIZARD_STYLE "Modern")
    SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_NAME})
    SET(CPACK_IFW_PACKAGE_PUBLISHER ${APP_PUBLISHER})
    SET(CPACK_IFW_PACKAGE_START_MENU_DIRECTORY ${APP_PUBLISHER})
    SET(CPACK_IFW_PRODUCT_URL ${APP_PATCHNOTE_URL})
    SET(CPACK_IFW_TARGET_DIRECTORY "@ApplicationsDirX64@/${PROJECT_NAME}")

    IF(WIN32)
        SET(APP_INSTALLER_EXTENSION ".exe")
    elseIF(APPLE)
        SET(APP_INSTALLER_EXTENSION ".dmg")
    endif()

    #icons
    SET(CPACK_IFW_PACKAGE_LOGO "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/app_64.png")
    SET(CPACK_IFW_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/package.ico")

##############
### INSTALL ##
##############

#install output content as is, probably overriding previous install() instructions
install(
    DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
    DESTINATION .
)

INCLUDE(CPack)

#configure default component
cpack_add_component(${CMAKE_INSTALL_DEFAULT_COMPONENT_NAME}
    HIDDEN
    REQUIRED
)

######################################
# CPACK IFW COMPONENTS CONFIGURATION #
######################################

#SET(CPACK_IFW_VERBOSE ON)
INCLUDE(CPackIFW)

#installer configuration
cpack_ifw_configure_component(${CMAKE_INSTALL_DEFAULT_COMPONENT_NAME}
    VIRTUAL
    FORCED_INSTALLATION
    SCRIPT "src/_ifw/install.js"
    USER_INTERFACES "src/_ifw/install.ui"
    #TRANSLATIONS "${CMAKE_BINARY_DIR}/fr.qm"
    DESCRIPTION ${APP_DESCRIPTION}
)

#repository for updates
cpack_ifw_add_repository(coreRepo 
    URL "https://dl.bintray.com/amphaal/rpgrpz"
)

########################
## ZIP FOR DEPLOYMENT ## 
########################

#source
SET(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}) #override as CPACK_SYSTEM_NAME may end up wrong (CMAKE bug?)
SET(APP_PACKAGED_IFW "${CMAKE_BINARY_DIR}/_CPack_Packages/${CPACK_SYSTEM_NAME}/IFW")
SET(APP_PACKAGED_PATH "${APP_PACKAGED_IFW}/${CPACK_PACKAGE_FILE_NAME}")
SET(APP_PACKAGED_INSTALLER_PATH "${APP_PACKAGED_PATH}${APP_INSTALLER_EXTENSION}")
SET(APP_PACKAGED_REPOSITORY_PATH "${APP_PACKAGED_PATH}/repository")

#create target to be invoked with bash
add_custom_target(zipForDeploy DEPENDS package)

#installer
add_custom_command(TARGET zipForDeploy
    COMMAND ${CMAKE_COMMAND} -E tar "c" "${CMAKE_BINARY_DIR}/installer.zip" "--format=zip" 
        ${APP_PACKAGED_INSTALLER_PATH}
    WORKING_DIRECTORY ${APP_PACKAGED_IFW}
    COMMENT "Ziping IFW installer..."
)
#repository
add_custom_command(TARGET zipForDeploy
    COMMAND ${CMAKE_COMMAND} -E tar "c" "${CMAKE_BINARY_DIR}/repository.zip" "--format=zip" 
        ${APP_PACKAGED_REPOSITORY_PATH}/Updates.xml
        ${APP_PACKAGED_REPOSITORY_PATH}/${PROJECT_NAME}
    WORKING_DIRECTORY ${APP_PACKAGED_REPOSITORY_PATH}
    COMMENT "Ziping IFW repository..."
)