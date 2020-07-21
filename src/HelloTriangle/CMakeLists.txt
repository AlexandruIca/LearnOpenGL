add_executable(HelloTriangle ${CMAKE_CURRENT_SOURCE_DIR}/hello_triangle.cpp)
target_link_libraries(HelloTriangle PRIVATE spdlog::spdlog SDL2::SDL2 glad::glad)
