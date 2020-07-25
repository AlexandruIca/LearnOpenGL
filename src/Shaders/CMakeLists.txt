add_executable(Shaders ${CMAKE_CURRENT_SOURCE_DIR}/shaders.cpp)
target_link_libraries(Shaders PRIVATE spdlog::spdlog SDL2::SDL2 glad::glad)
