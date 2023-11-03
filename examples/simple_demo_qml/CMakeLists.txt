cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)
project(gz-rendering-simple-demo-qml)

#------------------------------------------------------------------------
# Find gz-cmake
find_package(gz-cmake4 REQUIRED)
set(GZ_CMAKE_VER ${gz-cmake4_VERSION_MAJOR})

#------------------------------------------------------------------------
# Find gz-rendering
find_package(gz-rendering9)

#======================================
# Find Qt
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    Widgets
)
qt_standard_project_setup(REQUIRES 6.5)

#======================================
# Build

qt_add_executable(simple_demo_qml main.cc
  ogre_texture_import.cc ogre_texture_import.h
  ogre_texture_node.cc ogre_texture_node.h)

qt_add_qml_module(simple_demo_qml
  URI simple_demo_qml
  VERSION 1.0
  QML_FILES main.qml
)

set_target_properties(simple_demo_qml PROPERTIES
    # Prevent name clash with build subdirectory on case-insensitive file systems
    OUTPUT_NAME simple_demo_qml_app
)

target_link_libraries(simple_demo_qml PRIVATE
  Qt::Core
  Qt::Quick
  Qt::Qml
  Qt::Gui gz-rendering9::gz-rendering9)
