cmake_minimum_required(VERSION 3.22)
project(picker2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS
        Core Widgets
        REQUIRED)

add_executable(picker2 main.cpp MainWindow.cpp MainWindow.h MainWindow.ui)
target_link_libraries(picker2
        Qt::Core Qt::Widgets
        X11
        )
