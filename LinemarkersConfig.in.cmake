get_filename_component(PACKAGE_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

if(NOT TARGET linemarkers)
  include("${PACKAGE_CMAKE_DIR}/@LINEMARKERS_EXPORT@.cmake")
endif()
