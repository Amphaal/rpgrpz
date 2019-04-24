
#using pkgConfig, makes sure PKG_CONFIG_EXECUTABLE is set
find_package(PkgConfig REQUIRED)

#check requirements
if(NOT DEFINED GSTREAMER_LOCATION)
    message(FATAL_ERROR "GSTREAMER_LOCATION have not been set!")
endif()

#linking...
macro(LinkGStreamer)

    #add search path to pkgConfig searchpath
    list(APPEND CMAKE_PREFIX_PATH ${GSTREAMER_LOCATION})

    #base required modules
    pkg_check_modules(GST REQUIRED
        gstreamer-1.0
        libsoup-2.4
    )

    #plugins base...
        #plugins to include and dir
        pkg_get_variable(_GSTREAMER_PLUGINS
            "${GSTREAMER_LOCATION}/lib/pkgconfig/gstreamer-plugins-base-1.0.pc" 
        libraries)
        pkg_get_variable(_GSTREAMER_PLUGINS_DIR
            "${GSTREAMER_LOCATION}/lib/pkgconfig/gstreamer-plugins-base-1.0.pc" 
        pluginsdir)

        #add plugins to check in pkgconfig
        foreach(_gstPlugin IN ITEMS ${_GSTREAMER_PLUGINS})
            list(APPEND GST_PLUGINS_BASE_pc gstreamer-${_gstPlugin}-1.0)
        endforeach()

        #check for plugins
        pkg_check_modules(GST_PLUGINS_BASE REQUIRED 
            ${GST_PLUGINS_BASE_pc}
        )

    #linking
    link_directories(${GST_STATIC_LIBRARY_DIRS} ${GST_PLUGINS_BASE_STATIC_LIBRARY_DIRS})
    include_directories(${GST_STATIC_INCLUDE_DIRS} ${GST_PLUGINS_BASE_STATIC_INCLUDE_DIRS})
    #add_compile_options(${GST_STATIC_CFLAGS} ${GST_PLUGINS_BASE_STATIC_CFLAGS})

endmacro()


#copy libs in list at relative location
function(_GstThroughLibs target dllLocation libList dest)

    foreach(_gstLib IN ITEMS ${libList})

        #searched expression
        file(GLOB _se LIST_DIRECTORIES OFF
            "${dllLocation}/${_gstLib}*.dll"
        )
        
        #if .dll is not found, search with prefix
        if (_se STREQUAL "")
            
            #prefix search
            file(GLOB _se LIST_DIRECTORIES OFF
                "${dllLocation}/lib${_gstLib}*.dll"
            )

            #if still not found...
            if (_se STREQUAL "")
                #message(STATUS "\"${_gstLib}\" not found")
                continue()
            endif()

        endif()

        #extract lib name
        get_filename_component(_se_dll ${_se} NAME)

        #copy lib
        add_custom_command(TARGET ${target} COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            ${_se} $<TARGET_FILE_DIR:${target}>/${dest}${_se_dll}
        )
        #message(STATUS "\"${_gstLib}\" added")

    endforeach()

endfunction()


#copying dlls
macro(DeployGStreamer target plugins)

    #get the DLL lib location for base
    pkg_get_variable(_GSTREAMER_DLL_LOCATION 
        "${GSTREAMER_LOCATION}/lib/pkgconfig/gstreamer-1.0.pc" 
    toolsdir)

    #get the DLL lib location for plugins
    pkg_get_variable(_GSTREAMER_DLL_PLUGINS_LOCATION 
        "${GSTREAMER_LOCATION}/lib/pkgconfig/gstreamer-1.0.pc" 
    pluginsdir)

    #import
    _GstThroughLibs(${target} ${_GSTREAMER_DLL_LOCATION} "${GST_STATIC_LIBRARIES}" "/") #iterate
    _GstThroughLibs(${target} ${_GSTREAMER_DLL_LOCATION} "${GST_PLUGINS_BASE_STATIC_LIBRARIES}" "/") #iterate
    _GstThroughLibs(${target} ${_GSTREAMER_DLL_PLUGINS_LOCATION} "${plugins}" "/gst-plugins/") #iterate

endmacro()