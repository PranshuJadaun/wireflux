cmake_minimum_required(VERSION 3.19)
project(WireFlux VERSION 2.0 LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets SerialPort Charts Network)

qt_standard_project_setup()

qt_add_executable(WireFlux
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    serialdevice.h
    serialdevice.cpp
    dialog.h dialog.cpp dialog.ui
    config.h config.cpp config.ui
    resources.qrc
)

target_compile_definitions(WireFlux PRIVATE APP_VERSION="${PROJECT_VERSION}")
target_link_libraries(WireFlux
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::SerialPort
        Qt6::Charts
        Qt6::Network
)

include(GNUInstallDirs)

install(TARGETS WireFlux
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET WireFlux
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})