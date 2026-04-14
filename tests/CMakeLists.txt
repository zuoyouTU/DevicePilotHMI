add_library(test_common INTERFACE)

target_include_directories(test_common INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

target_link_libraries(test_common INTERFACE
    Qt6::Test
    project_options
    project_warnings
)

add_executable(settings_validation_tests
    settings_validation_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_validation.cpp
)

target_link_libraries(settings_validation_tests PRIVATE test_common)

add_test(NAME settings_validation_tests COMMAND settings_validation_tests)

add_executable(settings_json_codec_tests
    settings_json_codec_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_json_codec.cpp
)

target_link_libraries(settings_json_codec_tests PRIVATE test_common)

add_test(NAME settings_json_codec_tests COMMAND settings_json_codec_tests)

add_executable(settings_file_store_tests
    settings_file_store_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_file_store.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_json_codec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_validation.cpp
)

target_link_libraries(settings_file_store_tests PRIVATE test_common)

add_test(NAME settings_file_store_tests COMMAND settings_file_store_tests)

add_executable(settings_apply_service_tests
    settings_apply_service_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/backend/machine_backend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_apply_service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_file_store.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_json_codec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_validation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/runtime/machine_runtime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/log/log_interface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/log/log_model.cpp
)

target_link_libraries(settings_apply_service_tests PRIVATE
    test_common
    Qt6::Qml
)

add_test(NAME settings_apply_service_tests COMMAND settings_apply_service_tests)

add_executable(settings_draft_tests
    settings_draft_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_draft.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_file_store.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_json_codec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_validation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/log/log_interface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/log/log_model.cpp
)

target_link_libraries(settings_draft_tests PRIVATE
    test_common
    Qt6::Qml
)

add_test(NAME settings_draft_tests COMMAND settings_draft_tests)

add_executable(settings_session_tests
    settings_session_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/backend/machine_backend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_session.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_draft.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_apply_service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_file_store.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_json_codec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_validation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/runtime/machine_runtime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/log/log_interface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/log/log_model.cpp
)

target_link_libraries(settings_session_tests PRIVATE
    test_common
    Qt6::Qml
)

add_test(NAME settings_session_tests COMMAND settings_session_tests)

add_executable(alarm_manager_tests
    alarm_manager_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/backend/machine_backend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/alarm/alarm_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/runtime/machine_runtime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_file_store.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_json_codec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/settings/settings_validation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/log/log_interface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/log/log_model.cpp
)

target_link_libraries(alarm_manager_tests PRIVATE
    test_common
    Qt6::Qml
)

add_test(NAME alarm_manager_tests COMMAND alarm_manager_tests)
