find_package(Doxygen)

set(IGNITION_DOXYGEN_TAGFILES
    "\"${CMAKE_SOURCE_DIR}/doc/ogre_1-9.tag.xml.tgz=https://www.ogre3d.org/docs/api/1.9/\"")

if (DOXYGEN_FOUND)
  configure_file(${IGNITION_CMAKE_DOXYGEN_DIR}/api.in
                 ${CMAKE_BINARY_DIR}/api.dox @ONLY)

  configure_file(${IGNITION_CMAKE_DOXYGEN_DIR}/tutorials.in
                 ${CMAKE_BINARY_DIR}/tutorials.dox @ONLY)

  add_custom_target(doc ALL
    # Generate the API documentation
    ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/api.dox
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}

    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_BINARY_DIR}/tutorials.dox

    COMMENT "Generating API documentation with Doxygen" VERBATIM)

  install(FILES ${CMAKE_BINARY_DIR}/doc/${PROJECT_NAME_LOWER}.tag.xml
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/ignition/${PROJECT_NAME_LOWER}_${PROJECT_VERSION_MINOR})
endif()
