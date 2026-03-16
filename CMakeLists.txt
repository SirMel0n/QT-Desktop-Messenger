cmake_minimum_required(VERSION 3.19)
project(DesktopMessenger LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Gui Sql Network)

qt_standard_project_setup()

qt_add_executable(DesktopMessenger
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    Authentication/authentication.h Authentication/authentication.cpp Authentication/authentication.ui
    Registration/registration.h Registration/registration.cpp Registration/registration.ui
    TcpSocket.h TcpSocket.cpp
    ConfigManager.h
)

target_link_libraries(DesktopMessenger
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
        Qt6::Gui
        Qt6::Network
)

include(GNUInstallDirs)

install(TARGETS DesktopMessenger
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Copy server_config.ini to build directory
add_custom_command(TARGET DesktopMessenger POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/socket_config.ini"
        "$<TARGET_FILE_DIR:DesktopMessenger>/socket_config.ini"
    COMMENT "Copying socket_config.ini to build directory"
)

qt_generate_deploy_app_script(
    TARGET DesktopMessenger
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
install(FILES socket_config.ini DESTINATION ${CMAKE_INSTALL_BINDIR})
