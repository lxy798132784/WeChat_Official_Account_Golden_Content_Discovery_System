cmake_minimum_required(VERSION 3.22)
project(PremiumContentRadar VERSION 1.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Widgets Sql Network Test)

add_library(radar_core STATIC
    include/content_record.h
    include/IContentProvider.h
    include/PremiumContentFilterProxyModel.h src/PremiumContentFilterProxyModel.cpp
    include/AppSettings.h src/AppSettings.cpp
    include/BridgePayloadClient.h src/BridgePayloadClient.cpp
    include/DatabaseController.h src/DatabaseController.cpp
    include/ProxyTrafficBridge.h src/ProxyTrafficBridge.cpp
    include/PluginManager.h src/PluginManager.cpp
    include/ExportController.h src/ExportController.cpp
)
target_include_directories(radar_core PUBLIC include)
set_target_properties(radar_core PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(radar_core PUBLIC Qt6::Core Qt6::Widgets Qt6::Sql Qt6::Network)

add_library(WeChatProviderPlugin SHARED
    include/WeChatProviderPlugin.h src/WeChatProviderPlugin.cpp
    include/AdbAutomationEngine.h src/AdbAutomationEngine.cpp
    include/WeChatConfigWidget.h src/WeChatConfigWidget.cpp
    ui/wechat_config_widget.ui
)
target_include_directories(WeChatProviderPlugin PRIVATE include)
target_link_libraries(WeChatProviderPlugin PRIVATE radar_core Qt6::Core Qt6::Widgets Qt6::Sql Qt6::Network)
set_target_properties(WeChatProviderPlugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)

add_executable(premium-content-radar
    src/main.cpp
    include/MainWindow.h src/MainWindow.cpp ui/mainwindow.ui
    include/DashboardWidget.h src/DashboardWidget.cpp ui/dashboard_widget.ui
    include/ControlPanelWidget.h src/ControlPanelWidget.cpp ui/control_panel_widget.ui
    include/DataViewerWidget.h src/DataViewerWidget.cpp ui/data_viewer_widget.ui
    include/SeedManagerWidget.h src/SeedManagerWidget.cpp
    include/RuntimeLogWidget.h src/RuntimeLogWidget.cpp
    include/WeChatConfigWidget.h src/WeChatConfigWidget.cpp
)
target_include_directories(premium-content-radar PRIVATE include ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(premium-content-radar PRIVATE radar_core Qt6::Core Qt6::Widgets Qt6::Sql Qt6::Network)

include(CTest)
enable_testing()
add_executable(radar_tests tests/test_core.cpp)
target_include_directories(radar_tests PRIVATE include)
target_link_libraries(radar_tests PRIVATE radar_core Qt6::Core Qt6::Widgets Qt6::Sql Qt6::Network Qt6::Test)
add_test(NAME radar_tests COMMAND radar_tests)
set_tests_properties(radar_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

include(GNUInstallDirs)
install(TARGETS premium-content-radar RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS WeChatProviderPlugin LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/premium-content-radar/plugins)
install(FILES README.md LICENSE CHANGELOG.md SECURITY.md CONTRIBUTING.md CODE_OF_CONDUCT.md DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(DIRECTORY docs/ DESTINATION ${CMAKE_INSTALL_DOCDIR}/docs)
