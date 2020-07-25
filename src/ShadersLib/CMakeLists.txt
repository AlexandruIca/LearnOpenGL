add_executable(ShadersLib ${CMAKE_CURRENT_SOURCE_DIR}/shaders.cpp)
target_link_libraries(ShadersLib PRIVATE spdlog::spdlog SDL2::SDL2 glad::glad util)
copy_file(shader.vs ShadersLib)
copy_file(shader.fs ShadersLib)
