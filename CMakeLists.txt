cmake_minimum_required(VERSION 3.16)

project(DevicePilotHMI VERSION 0.1 LANGUAGES CXX)

add_library(project_options INTERFACE)
add_library(project_warnings INTERFACE)

target_compile_features(project_options INTERFACE cxx_std_20)
target_compile_options(project_warnings INTERFACE
    -Wall
    -Wextra
    -Wpedantic
)

if (MINGW)
    target_compile_options(project_warnings INTERFACE
        -Wa,-mbig-obj
    )
endif()

find_package(Qt6 REQUIRED COMPONENTS Quick Qml)

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

find_package(Qt6 REQUIRED COMPONENTS Quick Qml Test)

include(CTest)
enable_testing()

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()
