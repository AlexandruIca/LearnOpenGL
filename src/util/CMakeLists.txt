add_library(util STATIC ${CMAKE_CURRENT_SOURCE_DIR}/shader.cpp)
target_include_directories(util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_link_libraries(util PUBLIC glad::glad spdlog::spdlog glm::glm)
