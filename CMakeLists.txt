cmake_minimum_required(VERSION 3.24)

project(wxlib VERSION 0.1.0 LANGUAGES CXX)

if(NOT TARGET wx::wxmono)
    find_package(wxWidgets CONFIG REQUIRED COMPONENTS mono)
endif()

add_library(wxlib STATIC
    src/bar.cpp
    src/button.cpp
    src/combo.cpp
    src/container.cpp
    src/custom.cpp
    src/data.cpp
    src/edit.cpp
    src/graph.cpp
    src/grid.cpp
    src/menu.cpp
    src/project.cpp
    src/slider.cpp
    src/tab.cpp
    src/text.cpp
    src/window.cpp
)
add_library(wxlib::wxlib ALIAS wxlib)

target_compile_features(wxlib PUBLIC cxx_std_17)
target_include_directories(wxlib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(wxlib PUBLIC wx::wxmono)

if(MSVC)
    target_compile_options(wxlib PRIVATE /W4 /FS /utf-8)
endif()
