##########################
### CPACK CONFIGURATION ##
##########################

    SET(CPACK_GENERATOR IFW)

    #if cross compiling windows, use wrappes
    IF(CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
        SET(CPACK_IFW_FRAMEWORK_VERSION_FORCED "3.2.0")
        SET(CPACK_IFW_BINARYCREATOR_EXECUTABLE  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/wrappers/ifw/xbinarycreator.sh)
        SET(CPACK_IFW_REPOGEN_EXECUTABLE        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/wrappers/ifw/xrepogen.sh)
        SET(CPACK_IFW_INSTALLERBASE_EXECUTABLE  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/wrappers/ifw/xinstallerbase.sh)
        SET(CPACK_IFW_DEVTOOL_EXECUTABLE        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/wrappers/ifw/xdevtool.sh)
    ENDIF()

    SET(APP_DESCRIPTION ${PROJECT_NAME}
        fr "L'experience JdR simplifiée."
    )

    SET(CPACK_IFW_PACKAGE_WIZARD_STYLE "Modern")
    SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_NAME})
    SET(CPACK_IFW_PACKAGE_PUBLISHER ${APP_PUBLISHER})
    SET(CPACK_IFW_PACKAGE_START_MENU_DIRECTORY ${APP_PUBLISHER})
    SET(CPACK_IFW_PRODUCT_URL ${APP_PATCHNOTE_URL})
    SET(CPACK_IFW_TARGET_DIRECTORY "@ApplicationsDirX64@/${PROJECT_NAME}")

    IF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        SET(CPACK_IFW_PACKAGE_FILE_EXTENSION ".exe")
    elseif(APPLE)
        SET(CPACK_IFW_PACKAGE_FILE_EXTENSION ".dmg")
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
    PATTERN "*.pdb" EXCLUDE
)

INCLUDE(CPack)

#configure default component
cpack_add_component(${CMAKE_INSTALL_DEFAULT_COMPONENT_NAME})

######################################
# CPACK IFW COMPONENTS CONFIGURATION #
######################################

#SET(CPACK_IFW_VERBOSE ON)
INCLUDE(CPackIFW)

# #installer configuration
cpack_ifw_configure_component(${CMAKE_INSTALL_DEFAULT_COMPONENT_NAME}
    FORCED_INSTALLATION
    SCRIPT "src/_ifw/install.js"
    USER_INTERFACES "src/_ifw/install.ui"
    #TRANSLATIONS "${CMAKE_BINARY_DIR}/fr.qm"
    DESCRIPTION ${APP_DESCRIPTION}
)

#repository for updates
cpack_ifw_add_repository(coreRepo 
    URL "https://dl.bintray.com/amphaal/rpgrpz/ifw-${CPACK_SYSTEM_NAME}"
)

########################
## ZIP FOR DEPLOYMENT ## 
########################

#source
SET(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}) #override as CPACK_SYSTEM_NAME may end up wrong (CMAKE bug?)
SET(CPACK_PACKAGE_FILE_NAME_FULL ${CPACK_PACKAGE_FILE_NAME}${CPACK_IFW_PACKAGE_FILE_EXTENSION})

SET(APP_REPOSITORY ${CMAKE_BINARY_DIR}/_CPack_Packages/${CPACK_SYSTEM_NAME}/IFW/${CPACK_PACKAGE_FILE_NAME}/repository)

SET(APP_PACKAGE_LATEST ${CPACK_PACKAGE_NAME}-latest-${CPACK_SYSTEM_NAME})
SET(APP_PACKAGE_LATEST_FULL ${APP_PACKAGE_LATEST}${CPACK_IFW_PACKAGE_FILE_EXTENSION})

#create target to be invoked with bash
add_custom_target(zipForDeploy DEPENDS package)

#installer
add_custom_command(TARGET zipForDeploy
    COMMAND ${CMAKE_COMMAND} -E rename ${CPACK_PACKAGE_FILE_NAME_FULL} ${APP_PACKAGE_LATEST_FULL}
    COMMAND ${CMAKE_COMMAND} -E tar c installer.zip --format=zip ${APP_PACKAGE_LATEST_FULL}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Ziping IFW installer..."
)
# #repository
add_custom_command(TARGET zipForDeploy
    COMMAND ${CMAKE_COMMAND} -E tar c ${CMAKE_BINARY_DIR}/repository.zip --format=zip 
        Updates.xml
        ${CMAKE_INSTALL_DEFAULT_COMPONENT_NAME}
    WORKING_DIRECTORY ${APP_REPOSITORY}
    COMMENT "Ziping IFW repository..."
)