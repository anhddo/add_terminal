file(GLOB_RECURSE TWSADAPTER_SOURCES "*.cpp")
file(GLOB_RECURSE TWSADAPTER_HEADERS "*.h")

add_library(twsadapter SHARED ${TWSADAPTER_SOURCES} ${TWSADAPTER_HEADERS})
set_target_properties(twsadapter PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_include_directories(twsadapter
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC  ${CMAKE_SOURCE_DIR}/third_party/ibkr_api/client
    PRIVATE ${CMAKE_SOURCE_DIR}/third_party/ibkr_api
    PRIVATE ${CMAKE_SOURCE_DIR}/chart_glfw
)

target_link_libraries(twsadapter PUBLIC twsapi ibkr_proto)
