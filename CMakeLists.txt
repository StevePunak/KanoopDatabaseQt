cmake_minimum_required(VERSION 3.16)
include(GNUInstallDirs)

set(PROJ KanoopDatabaseQt)
project(${PROJ})

find_package(Qt6 REQUIRED COMPONENTS Core Sql)
qt_standard_project_setup()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(KANOOP_DATABASE_VERSION "1.1.1")

file(GLOB_RECURSE PROJ_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.qrc"
)

qt_add_library(${PROJ} ${PROJ_SOURCES})

target_compile_options(${PROJ} PRIVATE -Wextra -Wall -Werror)

set_target_properties(${PROJ} PROPERTIES VERSION "${KANOOP_DATABASE_VERSION}")
set_target_properties(${PROJ} PROPERTIES SOVERSION 1)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libKanoopDatabaseQt.pc.in" "libKanoopDatabaseQt.pc" @ONLY)

target_include_directories(${PROJ} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJ} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/Kanoop/database)
target_include_directories(${PROJ} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

get_directory_property(PARENT_DIR PARENT_DIRECTORY)
target_include_directories(${PROJ} PRIVATE ${PARENT_DIR}/KanoopCommon/include)

target_link_libraries(${PROJ} PRIVATE PRIVATE Qt6::Core Qt6::Sql KanoopCommonQt)

add_compile_definitions(KANOOP_QTGUI_LIBRARY)
add_compile_definitions(QT_DEPRECATED_WARNINGS)
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060000)  # Disables all the APIs deprecated before Qt 6.0.0

# Set install prefix.  This is necessary when installing without KYBER_OUTPUT_PREFIX set as this causes the path to start with two
# forward-slashes which CMake interprets to mean a network path.  Thus, we only include the "/" if there is something ahead of it.
if (DEFINED $ENV{KYBER_OUTPUT_PREFIX})
    set(KANOOP_INSTALL_PREFIX "$ENV{KYBER_OUTPUT_PREFIX}/")
endif()

install(TARGETS ${PROJ} LIBRARY DESTINATION "${KANOOP_INSTALL_PREFIX}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/Kanoop" DESTINATION "${KANOOP_INSTALL_PREFIX}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}" FILES_MATCHING PATTERN "*.h")
