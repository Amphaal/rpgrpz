#linking...
macro(FindGStreamer)

    #base required modules
    pkg_check_modules(Gst REQUIRED IMPORTED_TARGET
        gstreamer-1.0
        gstreamer-controller-1.0
        gstreamer-net-1.0
    )

    #plugins base...
        #plugins to include and dir
        pkg_get_variable(_GSTREAMER_PLUGINS
            gstreamer-plugins-base-1.0
        libraries)

        #add plugins to check in pkgconfig
        foreach(_gstPlugin IN ITEMS ${_GSTREAMER_PLUGINS})
            list(APPEND GST_PLUGINS_BASE_pc gstreamer-${_gstPlugin}-1.0)
        endforeach()

        #check for plugins
        pkg_check_modules(GstPluginsBase REQUIRED IMPORTED_TARGET 
            ${GST_PLUGINS_BASE_pc}
        )

endmacro()


#copy libs in list at relative location
function(_GstThroughLibs target dllLocation libList dest)

    foreach(_gstLib IN ITEMS ${libList})

        #searched expression
        file(GLOB _se LIST_DIRECTORIES OFF
            "${dllLocation}/${_gstLib}*${CMAKE_SHARED_LIBRARY_SUFFIX}"
        )
        
        #if shared lib is not found, search with prefix
        if (_se STREQUAL "")
            
            #prefix search
            file(GLOB _se LIST_DIRECTORIES OFF
                "${dllLocation}/lib${_gstLib}*${CMAKE_SHARED_LIBRARY_SUFFIX}"
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
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                    ${_libFile} $<TARGET_FILE_DIR:${target}>${dest}${_dllName}
            )
            #message(STATUS "\"${_gstLib}\" added")

        endforeach(_libFile)
        
    endforeach(_gstLib)

endfunction()


#copying dlls
macro(DeployGStreamer target plugins)

    #get the DLL lib location for base
    pkg_get_variable(_GSTREAMER_DLL_LOCATION gstreamer-1.0 toolsdir)

    #get the DLL lib location for plugins
    pkg_get_variable(_GSTREAMER_DLL_PLUGINS_LOCATION gstreamer-1.0 pluginsdir)

    #import
    _GstThroughLibs(${target} ${_GSTREAMER_DLL_LOCATION} "${Gst_STATIC_LIBRARIES}" "/") #iterate
    _GstThroughLibs(${target} ${_GSTREAMER_DLL_LOCATION} "${GstPluginsBase_STATIC_LIBRARIES}" "/") #iterate

    #plugins import
    SET(_plug_subdir "/gst-plugins/")
    
    #if request all plugins
    if ("all" IN_LIST plugins)

        #find all files 
        message("all plugins requested !")
        file(GLOB _temp_plugins LIST_DIRECTORIES OFF
        "${_GSTREAMER_DLL_PLUGINS_LOCATION}/*${CMAKE_SHARED_LIBRARY_SUFFIX}"
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