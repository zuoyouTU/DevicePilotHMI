cmake_minimum_required(VERSION 3.25)

project(DevicePilotHMI VERSION 0.1 LANGUAGES CXX)

include(GNUInstallDirs)

if (NOT CMAKE_PREFIX_PATH AND DEFINED ENV{QT_ROOT_DIR} AND NOT "$ENV{QT_ROOT_DIR}" STREQUAL "")
    file(TO_CMAKE_PATH "$ENV{QT_ROOT_DIR}" DEVICEPILOTHMI_QT_ROOT_DIR)
    list(PREPEND CMAKE_PREFIX_PATH "${DEVICEPILOTHMI_QT_ROOT_DIR}")
endif()

add_library(project_options INTERFACE)
add_library(project_warnings INTERFACE)

target_compile_features(project_options INTERFACE cxx_std_20)
target_compile_options(project_warnings INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wextra>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wpedantic>
)

if (MINGW)
    target_compile_options(project_warnings INTERFACE
        -Wa,-mbig-obj
    )
endif()

find_package(Qt6 QUIET COMPONENTS Quick Qml Test)

if (NOT Qt6_FOUND)
    message(FATAL_ERROR
        "Qt 6.11+ was not found.\n"
        "\n"
        "How to fix it:\n"
        "  - Command line on Linux: run ./scripts/build.sh --preset linux-ninja-debug --qt-root /path/to/Qt/6.11.0/gcc_64\n"
        "  - Qt Creator on Linux: set QT_ROOT_DIR or CMAKE_PREFIX_PATH to your Qt 6 root before opening the project,\n"
        "    or create a local CMakeUserPresets.json based on CMakeUserPresets.json.example.\n"
        "  - Windows: run ./scripts/setup_windows_qt_env.ps1 first.\n"
        "\n"
        "Expected Qt root examples:\n"
        "  - Linux: /home/<user>/Qt/6.11.0/gcc_64\n"
        "  - Windows MinGW: C:/Qt/6.11.0/mingw_64\n"
        "  - Windows MSVC: C:/Qt/6.11.0/msvc2022_64\n"
    )
endif()

qt_standard_project_setup(REQUIRES 6.11)

qt_add_executable(DevicePilotHMI
    src/main.cpp
)

qt_add_qml_module(DevicePilotHMI
    URI DevicePilotHMI
    VERSION 1.0

    SOURCES
        src/backend/machine_backend.h
        src/backend/machine_backend.cpp
        src/backend/simulated_machine_backend.h
        src/backend/simulated_machine_backend.cpp
        src/log/log_model.h
        src/log/log_model.cpp
        src/log/log_filter_proxy_model.h
        src/log/log_filter_proxy_model.cpp
        src/log/log_interface.h
        src/log/log_interface.cpp
        src/settings/settings_defined.h
        src/settings/settings_manager.h
        src/settings/settings_manager.cpp
        src/settings/settings_draft.h
        src/settings/settings_draft.cpp
        src/settings/settings_session.h
        src/settings/settings_session.cpp
        src/runtime/machine_runtime.h
        src/runtime/machine_runtime.cpp
        src/runtime/machine_types.h
        src/alarm/alarm_manager.h
        src/alarm/alarm_manager.cpp
        src/settings/settings_file_store.cpp
        src/settings/settings_file_store.h
        src/settings/settings_json_codec.cpp
        src/settings/settings_json_codec.h
        src/settings/settings_validation.cpp
        src/settings/settings_validation.h
        src/settings/settings_apply_service.cpp
        src/settings/settings_apply_service.h



    QML_FILES
        qml/Main.qml
        qml/pages/DashboardPage.qml
        qml/pages/LogPage.qml
        qml/pages/SettingsPage.qml
        qml/components/StatusBanner.qml
        qml/components/MetricCard.qml
        qml/components/ControlPanel.qml
)

set_target_properties(DevicePilotHMI PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_include_directories(DevicePilotHMI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/log
    ${CMAKE_CURRENT_SOURCE_DIR}/src/alarm
    ${CMAKE_CURRENT_SOURCE_DIR}/src/runtime
    ${CMAKE_CURRENT_SOURCE_DIR}/src/settings
)

target_link_libraries(DevicePilotHMI
    PRIVATE
        Qt6::Quick
        Qt6::Qml
        project_options
        project_warnings
)

install(TARGETS DevicePilotHMI
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET DevicePilotHMI
    OUTPUT_SCRIPT devicepilothmi_deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)

install(SCRIPT ${devicepilothmi_deploy_script})

include(CTest)
enable_testing()

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
