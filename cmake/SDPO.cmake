MACRO(SDP_ADD_GUI_SUBDIR_SOURCES ...)
        SET(prefix ${ARGV0})
        SET(dir ${ARGV1})
        ADD_SUBDIRECTORY(${dir})
        SET(sources "")
        SET(headers "")
        SET(defs "")
        SET(mocs "")
        SET(uis "")
        SET(resources "")
        GET_PROPERTY(sources DIRECTORY ${dir} PROPERTY SOURCES)
        GET_PROPERTY(headers DIRECTORY ${dir} PROPERTY HEADERS)
        GET_PROPERTY(defs DIRECTORY ${dir} PROPERTY DEFS)
        GET_PROPERTY(mocs DIRECTORY ${dir} PROPERTY MOCS)
        GET_PROPERTY(uis DIRECTORY ${dir} PROPERTY UIS)
        GET_PROPERTY(resources DIRECTORY ${dir} PROPERTY RESOURCES)

        ADD_DEFINITIONS(${defs})
    	SET(${prefix}_DEFINITION ${defs})

        FOREACH (_src ${sources})
                SET(_src ${dir}/${_src})
                SET(${prefix}_SOURCES ${${prefix}_SOURCES} ${_src})
        ENDFOREACH(_src)

        FILE(RELATIVE_PATH _package_dir ${SDP_SOURCE_DIR}/lib ${CMAKE_CURRENT_SOURCE_DIR})

        FOREACH (_head ${headers})
                SET(_head ${dir}/${_head})
                SET(${prefix}_HEADERS ${${prefix}_HEADERS} ${_head})
                IF(NOT ARGV2)
                        INSTALL(FILES ${_head} DESTINATION ${SDP_INCLUDE_DIR}/${_package_dir}/${dir})
                ENDIF(NOT ARGV2)
        ENDFOREACH(_head)

        FOREACH (_moc ${mocs})
                SET(_moc ${dir}/${_moc})
                SET(${prefix}_MOC_HEADERS ${${prefix}_MOC_HEADERS} ${_moc})
                IF(NOT ARGV2)
                        INSTALL(FILES ${_moc} DESTINATION ${SDP_INCLUDE_DIR}/${_package_dir}/${dir})
                ENDIF(NOT ARGV2)
        ENDFOREACH(_moc)

        FOREACH (_res ${resources})
                SET(_res ${dir}/${_res})
                SET(${prefix}_RESOURCES ${${prefix}_RESOURCES} ${_res})
        ENDFOREACH(_res)

        FOREACH (_ui ${uis})
                SET(_ui ${dir}/${_ui})
                SET(${prefix}_UI ${${prefix}_UI} ${_ui})
		# TODO: Add QT4_WRAP_HERE and set the install directory here as well
		# in cmakelists.txt set UI_HEADERS to ""
		SET(_ui_out "")
		SDP_QT4_WRAP_UI(_ui_out ${_ui})
		IF(NOT ARGV2)
			INSTALL(FILES ${_ui_out} DESTINATION ${SDP_INCLUDE_DIR}/${_package_dir}/${dir})
		ENDIF(NOT ARGV2)
        ENDFOREACH(_ui)
ENDMACRO(SDP_ADD_GUI_SUBDIR_SOURCES)


MACRO(SDP_SETUP_GUI_LIB_SUBDIR _package)
        SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY SOURCES ${${_package}_SOURCES})
        SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY HEADERS ${${_package}_HEADERS})
        SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY DEFS ${${_package}_DEFINITIONS})
        SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY MOCS ${${_package}_MOC_HEADERS})
        SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY UIS ${${_package}_UI})
        SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY RESOURCES ${${_package}_RESOURCES})
ENDMACRO(SDP_SETUP_GUI_LIB_SUBDIR)


# This macro changed in CMake 2.8 it does not work anymore correctly so
# it is included here as _COMPAT version
MACRO (QT4_EXTRACT_OPTIONS_COMPAT _qt4_files _qt4_options)
  SET(${_qt4_files})
  SET(${_qt4_options})
  SET(_QT4_DOING_OPTIONS FALSE)
  FOREACH(_currentArg ${ARGN})
    IF ("${_currentArg}" STREQUAL "OPTIONS")
      SET(_QT4_DOING_OPTIONS TRUE)
    ELSE ("${_currentArg}" STREQUAL "OPTIONS")
      IF(_QT4_DOING_OPTIONS) 
        LIST(APPEND ${_qt4_options} "${_currentArg}")
      ELSE(_QT4_DOING_OPTIONS)
        LIST(APPEND ${_qt4_files} "${_currentArg}")
      ENDIF(_QT4_DOING_OPTIONS)
    ENDIF ("${_currentArg}" STREQUAL "OPTIONS")
  ENDFOREACH(_currentArg) 
ENDMACRO (QT4_EXTRACT_OPTIONS_COMPAT)


MACRO(SDP_QT4_WRAP_UI outfiles)
  QT4_EXTRACT_OPTIONS_COMPAT(ui_files ui_options ${ARGN})

  FOREACH (it ${ui_files})
    GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)
    GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
    GET_FILENAME_COMPONENT(_rel ${it} PATH)
    IF (_rel)
      SET(_rel "${_rel}/")
    ENDIF (_rel)
    SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/${_rel}ui_${outfile}.h) # Here we set output
    ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
      COMMAND ${QT_UIC_EXECUTABLE}
      ARGS ${ui_options} -o ${outfile} ${infile}
      MAIN_DEPENDENCY ${infile})
    SET(${outfiles} ${${outfiles}} ${outfile})
  ENDFOREACH (it)
ENDMACRO(SDP_QT4_WRAP_UI)


MACRO(SDP_ADD_GUI_LIBRARY_CUSTOM_INSTALL _library_package _library_name)
	INCLUDE(${QT_USE_FILE})

	SET(_global_library_package SC_${_library_package})

	IF(SHARED_LIBRARIES)
		IF (WIN32)
			ADD_DEFINITIONS(-D${_global_library_package}_EXPORTS)
		ENDIF (WIN32)
		SET(${_library_package}_TYPE SHARED)
		SET(${_global_library_package}_SHARED 1)
	ENDIF(SHARED_LIBRARIES)

	# Create MOC Files
	IF (${_library_package}_MOC_HEADERS)
		QT4_WRAP_CPP(${_library_package}_MOC_SOURCES ${${_library_package}_MOC_HEADERS})
	ENDIF (${_library_package}_MOC_HEADERS)

	# Create UI Headers
	IF (${_library_package}_UI)
		SDP_QT4_WRAP_UI(${_library_package}_UI_HEADERS ${${_library_package}_UI})
		INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
		INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
	ENDIF (${_library_package}_UI)

	# Add resources
	IF (${_library_package}_RESOURCES)
		QT4_ADD_RESOURCES(${_library_package}_RESOURCE_SOURCES ${${_library_package}_RESOURCES})
	ENDIF (${_library_package}_RESOURCES)

	SET(LIBRARY ${_global_library_package})
	SET(LIBRARY_NAME ${_library_name})

#	IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake)
#		CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
#		               ${CMAKE_CURRENT_BINARY_DIR}/config.h)
#		CONFIGURE_FILE(${CMAKE_CURRENT_BINARY_DIR}/config.h
#		               ${CMAKE_CURRENT_BINARY_DIR}/config.h)
#		CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/win32api.h.cmake
#		               ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_API_H})
#		SET(${_library_package}_HEADERS
#			${${_library_package}_HEADERS}
#			${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_API_H}
#			${CMAKE_CURRENT_BINARY_DIR}/config.h)
#	ELSE (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake)
#		CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/win32api.h.cmake
#		               ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_API_H})
#		SET(${_library_package}_HEADERS
#			${${_library_package}_HEADERS}
#			${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_API_H})
#	ENDIF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake)

	SET(${_library_package}_FILES_
	    ${${_library_package}_SOURCES}
	    ${${_library_package}_MOC_SOURCES}
	    ${${_library_package}_UI_HEADERS}
	    ${${_library_package}_RESOURCE_SOURCES}
	)

	ADD_LIBRARY(sdp_${_library_name} ${${_library_package}_TYPE} ${${_library_package}_FILES_})
	TARGET_LINK_LIBRARIES(sdp_${_library_name} ${QT_LIBRARIES})
ENDMACRO(SDP_ADD_GUI_LIBRARY_CUSTOM_INSTALL)


MACRO(SDP_ADD_GUI_LIBRARY _library_package _library_name)
    ADD_DEFINITIONS(-W -Wall)
	SDP_ADD_GUI_LIBRARY_CUSTOM_INSTALL(${_library_package} ${_library_name})

	INSTALL(TARGETS sdp_${_library_name}
		RUNTIME DESTINATION ${SDP_BIN_DIR}
		ARCHIVE DESTINATION ${SDP_LIB_DIR}
		LIBRARY DESTINATION ${SDP_LIB_DIR}
	)
ENDMACRO(SDP_ADD_GUI_LIBRARY)


MACRO(SDP_LIB_INSTALL_HEADERS ...)
	IF(${ARGC} GREATER 1)
		SET(_package_dir "${ARGV1}")
	ELSE(${ARGC} GREATER 1)
		FILE(RELATIVE_PATH _package_dir ${SDP_SOURCE_DIR}/lib ${CMAKE_CURRENT_SOURCE_DIR})
	ENDIF(${ARGC} GREATER 1)

	INSTALL(FILES ${${ARGV0}_HEADERS}
		DESTINATION ${SDP_INCLUDE_DIR}/${_package_dir}
	)

	IF (${ARGV0}_MOC_HEADERS)
		INSTALL(FILES ${${ARGV0}_MOC_HEADERS}
			DESTINATION ${SDP_INCLUDE_DIR}/${_package_dir}
		)
	ENDIF (${ARGV0}_MOC_HEADERS)

	IF (${ARGV0}_UI_HEADERS)
		INSTALL(FILES ${${ARGV0}_UI_HEADERS}
			DESTINATION ${SDP_INCLUDE_DIR}/${_package_dir}
		)
	ENDIF (${ARGV0}_UI_HEADERS)
ENDMACRO(SDP_LIB_INSTALL_HEADERS)


MACRO(SDP_ADD_GUI_EXECUTABLE _package _name)
    ADD_DEFINITIONS(-W -Wall)
	INCLUDE(${QT_USE_FILE})
	INCLUDE_DIRECTORIES(${SDP_SOURCE_DIR}/lib)
	
	# Create MOC Files
	IF (${_package}_MOC_HEADERS)
		QT4_WRAP_CPP(${_package}_MOC_SOURCES ${${_package}_MOC_HEADERS})
	ENDIF (${_package}_MOC_HEADERS)

	# Create UI Headers
	IF (${_package}_UI)
		QT4_WRAP_UI(${_package}_UI_HEADERS ${${_package}_UI})
		INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
		INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
	ENDIF (${_package}_UI)

	# Add resources
	IF (${_package}_RESOURCES)
		QT4_ADD_RESOURCES(${_package}_RESOURCE_SOURCES ${${_package}_RESOURCES})
	ENDIF (${_package}_RESOURCES)
    
    SET(
		${_package}_FILES_
			${${_package}_SOURCES}
			${${_package}_MOC_SOURCES}
			${${_package}_UI_HEADERS}
			${${_package}_RESOURCE_SOURCES}
	)

	IF(WIN32)
		ADD_EXECUTABLE(${_name} WIN32 ${${_package}_FILES_})
		TARGET_LINK_LIBRARIES(${_name} ${QT_QTMAIN_LIBRARY})
	ELSE(WIN32)
		IF(APPLE)
            MESSAGE(STATUS "Adding Mac OSX Bundle in app executable")
            SET(${_name} MACOSX_BUNDLE)
#            SET(MACOSX_BUNDLE_GUI_IDENTIFIER "com.ipgp.\${PRODUCT_NAME:identifier}")
    		ADD_EXECUTABLE(${_name} MACOSX_BUNDLE ${${_package}_FILES_})
	    	TARGET_LINK_LIBRARIES(${_name} ${QT_QTMAIN_LIBRARY})
		ELSE(APPLE)
		    ADD_EXECUTABLE(${_name} ${${_package}_FILES_})
		ENDIF(APPLE)
	ENDIF(WIN32)
	TARGET_LINK_LIBRARIES(${_name} ${QT_LIBRARIES})
	TARGET_LINK_LIBRARIES(${_name} ${QT_QTOPENGL_LIBRARY})
	
	IF(APPLE)
    	# the install RPATH for bar to find foo in the install tree.
        # if the install RPATH is not provided, the install bar will have none
        SET_TARGET_PROPERTIES(${_name} PROPERTIES INSTALL_RPATH "@loader_path/../lib")
    ENDIF(APPLE)

	INSTALL(TARGETS ${_name}
		RUNTIME DESTINATION ${SDP_BIN_DIR}
		ARCHIVE DESTINATION ${SDP_LIB_DIR}
		LIBRARY DESTINATION ${SDP_LIB_DIR}
	)
ENDMACRO(SDP_ADD_GUI_EXECUTABLE)



MACRO(SDP_LINK_LIBRARIES _name)
	TARGET_LINK_LIBRARIES(${_name} ${ARGN})
ENDMACRO(SDP_LINK_LIBRARIES)


MACRO(SDP_LINK_LIBRARIES_INTERNAL _name)
	FOREACH(_lib ${ARGN})
		TARGET_LINK_LIBRARIES(${_name} sdp_${_lib})
	ENDFOREACH(_lib)
ENDMACRO(SDP_LINK_LIBRARIES_INTERNAL)


MACRO(SDP_LIB_LINK_LIBRARIES _library_name)
	TARGET_LINK_LIBRARIES(sdp_${_library_name} ${ARGN})
ENDMACRO(SDP_LIB_LINK_LIBRARIES)


MACRO(SDP_LIB_LINK_LIBRARIES_INTERNAL _library_name)
	FOREACH(_lib ${ARGN})
		TARGET_LINK_LIBRARIES(sdp_${_library_name} sdp_${_lib})
	ENDFOREACH(_lib)
ENDMACRO(SDP_LIB_LINK_LIBRARIES_INTERNAL)