cmake_minimum_required(VERSION 3.19)
project(VisualizerOfMathematicalFunctions LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets PrintSupport Svg)
qt_standard_project_setup()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC_PARALLEL ON)
set(CMAKE_MACOSX_RPATH ON)

add_compile_options(-O2 -pipe)

add_library(QCustomPlot STATIC
    qcustomplot/qcustomplot.cpp
    qcustomplot/qcustomplot.h
)
target_link_libraries(QCustomPlot PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport Qt6::Svg)
target_include_directories(QCustomPlot PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(VisualizerOfMathematicalFunctions
    WIN32 MACOSX_BUNDLE
    mainwindow.h
    mainwindow.cpp
    userwidget.h
    userwidget.cpp
    customtextedit.h
    customtextedit.cpp
    customscrollbar.h
    customscrollbar.cpp
    main.cpp
    resources/resources.qrc
    styleutils.cpp
    buildgraph.cpp
    parser.cpp
    createnewtextedit.cpp
)

target_include_directories(VisualizerOfMathematicalFunctions PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(VisualizerOfMathematicalFunctions PRIVATE QCustomPlot Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport Qt6::Svg)

target_precompile_headers(VisualizerOfMathematicalFunctions PRIVATE
    <QApplication>
    <QMainWindow>
    <QWidget>
    <QVector>
    <QString>
    qcustomplot/qcustomplot.h
)

set_target_properties(VisualizerOfMathematicalFunctions PROPERTIES UNITY_BUILD ON)

include(GNUInstallDirs)
install(TARGETS VisualizerOfMathematicalFunctions
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -march=native -flto)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

qt_generate_deploy_app_script(
    TARGET VisualizerOfMathematicalFunctions
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
