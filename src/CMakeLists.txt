macro(copy_file FILE_NAME EXECUTABLE_NAME)
  add_custom_command(
    TARGET ${EXECUTABLE_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${FILE_NAME})
endmacro()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/util/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/HelloTriangle/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Shaders/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ShadersLib/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Textures1/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Transformations/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/CoordinateSystems/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/CoordinateSystems2/)
