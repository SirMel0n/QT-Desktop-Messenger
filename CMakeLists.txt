cmake_minimum_required(VERSION 3.19)
project(DesktopMessenger LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Gui Sql)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(DesktopMessenger
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    Authentication/authentication.h Authentication/authentication.cpp Authentication/authentication.ui
    appwindow.h appwindow.cpp
    Registration/registration.h Registration/registration.cpp Registration/registration.ui
)

target_link_libraries(DesktopMessenger
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Sql
        Qt::Gui
)
target_link_libraries(DesktopMessenger PRIVATE Qt6::Widgets)

include(GNUInstallDirs)

install(TARGETS DesktopMessenger
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET DesktopMessenger
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
