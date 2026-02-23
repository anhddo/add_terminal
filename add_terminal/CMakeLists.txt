find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fix MSVC __cplusplus macro to report correct C++ standard
if(MSVC)
    add_compile_options(/Zc:__cplusplus /std:c++20)
endif()

set(TWS_SAMPLES_DIR ${CMAKE_SOURCE_DIR}/third_party/tws_api/samples/Cpp/TestCppClient)

add_executable(add_terminal
    main.cpp
    App.cpp
    App.h
    Config.h
    DataManager.h
    renderer.cpp
    renderer.h
    data_api/polygon_io.cpp
    data_api/polygon_io.h

    ibkr.cpp
    ibkr.h
    command.h
    event.h

    ${TWS_SAMPLES_DIR}/TestCppClient.cpp
    ${TWS_SAMPLES_DIR}/AccountSummaryTags.cpp
    ${TWS_SAMPLES_DIR}/AvailableAlgoParams.cpp
    ${TWS_SAMPLES_DIR}/ContractSamples.cpp
    ${TWS_SAMPLES_DIR}/OrderSamples.cpp
    ${TWS_SAMPLES_DIR}/ScannerSubscriptionSamples.cpp
    ${TWS_SAMPLES_DIR}/Utils.cpp
    ${TWS_SAMPLES_DIR}/StdAfx.cpp
)



target_include_directories(add_terminal
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/data_api
    PRIVATE ${CMAKE_SOURCE_DIR}/third_party/tws_api/source/cppclient/client
    PRIVATE ${TWS_SAMPLES_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/third_party/tws_api/source/cppclient
)

target_link_libraries(add_terminal
    PRIVATE twsapi
    PRIVATE glad
    PRIVATE glfw
    PRIVATE glm::glm
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE imgui::imgui
    PRIVATE opengl32
)

# Copy required DLLs to output directory
add_custom_command(TARGET add_terminal POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:twsapi>
        $<TARGET_FILE_DIR:add_terminal>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:protobuf::libprotobuf>
        $<TARGET_FILE_DIR:add_terminal>
    COMMENT "Copying DLL dependencies to output directory"
)

# Copy config files to output directory
add_custom_command(TARGET add_terminal POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/config.json.template
        $<TARGET_FILE_DIR:add_terminal>/config.json.template
    COMMENT "Copying config template"
)

# Copy config.json if it exists, otherwise copy from template
add_custom_command(TARGET add_terminal POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/config.json
        $<TARGET_FILE_DIR:add_terminal>/config.json
    COMMAND ${CMAKE_COMMAND} -E echo "Copied config.json"
    COMMENT "Copying config.json if it exists"
)

# If config.json doesn't exist in source, copy template as config.json
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/config.json")
    add_custom_command(TARGET add_terminal POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/config.json.template
            $<TARGET_FILE_DIR:add_terminal>/config.json
        COMMENT "Creating config.json from template"
    )
endif()

# Copy be_data.json if it exists
add_custom_command(TARGET add_terminal POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/be_data.json
        $<TARGET_FILE_DIR:add_terminal>/be_data.json
    COMMAND_EXPAND_LISTS
)
