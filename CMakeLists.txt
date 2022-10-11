cmake_minimum_required(VERSION 3.0)
project(vulkan_shaders)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

find_package(glfw3 3.3 REQUIRED)
find_package(Vulkan REQUIRED)

include_directories(
    /usr/local/include
    ${CMAKE_SOURCE_DIR}/include
)

add_library(vkbootstrap include/VkBootstrap.h include/VkBootstrapDispatch.h src/VkBootstrap.cpp)

link_libraries(
    glfw
    vulkan
    vkbootstrap
)

add_executable(main src/main.cpp)

# uncomment below lines to print all the variables
# get_cmake_property(_variableNames VARIABLES)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()