include(D:/MP-Desktop-Messenger/QT-Desktop-Messenger/out/build/debug/.qt/QtDeploySupport.cmake)
include("${CMAKE_CURRENT_LIST_DIR}/DesktopMessenger-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_ALL_MODULES_FOUND_VIA_FIND_PACKAGE "ZlibPrivate;EntryPointPrivate;Core;Gui;Widgets;Sql")

qt6_deploy_runtime_dependencies(
    EXECUTABLE D:/MP-Desktop-Messenger/QT-Desktop-Messenger/out/build/debug/DesktopMessenger.exe
    GENERATE_QT_CONF
)
