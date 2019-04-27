
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
        gstreamer-controller-1.0
        libsoup-2.4
        gnutls
        libmpg123
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

        #foreach libname found
        foreach(_libFile IN ITEMS ${_se})
            
            #extract lib name
            get_filename_component(_dllName ${_libFile} NAME)

            #copy lib
            add_custom_command(TARGET ${target} COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                ${_libFile} $<TARGET_FILE_DIR:${target}>${dest}${_dllName}
            )
            #message(STATUS "\"${_gstLib}\" added")

        endforeach(_libFile)
        
    endforeach(_gstLib)

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

    #plugins import
    SET(_plug_subdir "/gst-plugins/")
    
    #if request all plugins
    if (${plugins} STREQUAL "all")

        #find all files 
        message("all plugins requested !")
        file(GLOB _temp_plugins LIST_DIRECTORIES OFF
        "${_GSTREAMER_DLL_PLUGINS_LOCATION}/*.dll"
        )

        #get their names
        foreach(_t IN ITEMS ${_temp_plugins})
            get_filename_component(_fu ${_t} NAME_WE)
            list(APPEND _def_plugins "${_fu}")
        endforeach()
    else()
        SET(_def_plugins ${plugins})
    endif()

    #import...
    _GstThroughLibs(${target} ${_GSTREAMER_DLL_PLUGINS_LOCATION} "${_def_plugins}" ${_plug_subdir}) #iterate

endmacro()