add_library(cimgui_impl STATIC
    ${CMAKE_SOURCE_DIR}/third_party/cimgui/cimgui_impl.cpp
    ${CMAKE_SOURCE_DIR}/third_party/cimgui/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/third_party/cimgui/imgui/backends/imgui_impl_opengl3.cpp
)
target_compile_definitions(cimgui_impl PUBLIC
    CIMGUI_USE_GLFW
    CIMGUI_USE_OPENGL3
    GLFW_INCLUDE_NONE
)
target_include_directories(cimgui_impl PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/cimgui
    ${CMAKE_SOURCE_DIR}/third_party/cimgui/imgui
    ${CMAKE_SOURCE_DIR}/third_party/cimgui/imgui/backends
)
target_link_libraries(cimgui_impl PUBLIC cimgui glfw glad)

add_executable(c45k main.c)

target_compile_definitions(c45k PRIVATE CIMGUI_DEFINE_ENUMS_AND_STRUCTS)

target_include_directories(c45k PRIVATE
    ${CMAKE_SOURCE_DIR}/TwsAdapter
    ${CMAKE_SOURCE_DIR}/third_party/cimgui
    ${CMAKE_SOURCE_DIR}/third_party/glad/include
    ${CMAKE_SOURCE_DIR}/third_party/glfw
)

target_link_libraries(c45k PRIVATE twsadapter cimgui cimgui_impl glfw glad)

add_custom_command(TARGET c45k POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:twsadapter>
        $<TARGET_FILE_DIR:c45k>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<IF:$<CONFIG:Debug>,${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/debug/bin,${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/bin>"
        $<TARGET_FILE_DIR:c45k>
)
