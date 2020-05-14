cmake_minimum_required(VERSION 2.8.12)
project(linemarkers)

set(LINEMARKERS_VERSION "0.1.0")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system filesystem program_options)

add_library(linemarkers linemarkers.cxx)
target_include_directories(linemarkers
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
           $<INSTALL_INTERFACE:include>)

# Define public headers to get them automatically installed.
set_target_properties(linemarkers PROPERTIES
    PUBLIC_HEADER linemarkers.hxx)

add_executable(linemarkers-tool main.cxx)
set_target_properties(linemarkers-tool PROPERTIES
    OUTPUT_NAME linemarkers)

target_link_libraries(linemarkers-tool PRIVATE linemarkers Boost::program_options Boost::filesystem)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(LINEMARKERS_EXPORT LinemarkersTargets)

# Install the lib and its headers. Flag it for export.
install(TARGETS linemarkers linemarkers-tool
    EXPORT ${LINEMARKERS_EXPORT}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/linemarkers
    )

# Create the export file for the build tree.
export(TARGETS linemarkers FILE "${PROJECT_BINARY_DIR}/${LINEMARKERS_EXPORT}.cmake")
# Create the export file for the install tree.
install(EXPORT ${LINEMARKERS_EXPORT}
    DESTINATION "${CMAKE_INSTALL_PREFIX}")

# The config files could be generated using a macro (see [1]), but it seems
# to be more complex than to do it manually (see [2]).
# [1] https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html
# [2] https://gitlab.kitware.com/cmake/community/wikis/doc/tutorials/How-to-create-a-ProjectConfig.cmake-file

# Create the config file for the build tree.
set(CONF_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")
configure_file(LinemarkersConfig.in.cmake
    "${PROJECT_BINARY_DIR}/LinemarkersConfig.cmake" @ONLY)

# Create the config file for the install tree.
set(CONF_INCLUDE_DIRS "\${PACKAGE_CMAKE_DIR}/${CMAKE_INSTALL_INCLUDEDIR}")
configure_file(LinemarkersConfig.in.cmake
    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/LinemarkersConfig.cmake" @ONLY)

# Create the package version file.
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/LinemarkersConfigVersion.cmake"
    VERSION "${LINEMARKERS_VERSION}"
    COMPATIBILITY SameMajorVersion)

# Install both the config and version file.
install(FILES
    "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/LinemarkersConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/LinemarkersConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_PREFIX}")

include(CTest)

if(BUILD_TESTING)
    find_package(GTest REQUIRED NO_MODULE)
    enable_testing()

    add_executable(unit-tests tests.cxx)
    target_link_libraries(unit-tests PRIVATE GTest::gmock_main linemarkers Boost::system Boost::filesystem)
    add_test(unit-tests unit-tests)
endif()
