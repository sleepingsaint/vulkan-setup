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


link_libraries(glfw vulkan)

add_library(vkbootstrap include/VkBootstrap.h include/VkBootstrapDispatch.h src/VkBootstrap.cpp)
add_library(vkengine include/VkEngine.h include/VkBootstrap.h include/VkBootstrapDispatch.h include/VkInitializers.h src/VkInitializers.cpp src/VkBootstrap.cpp src/VkEngine.cpp)
add_library(vkpipeline include/VkPipelineBuilder.h src/VkPipelineBuilder.cpp)

link_libraries(
    vkbootstrap
    vkengine
    vkpipeline
)

add_executable(main src/main.cpp)

# download glslangValidator bin from https://github.com/KhronosGroup/glslang/releases/tag/master-tot
find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/shaders/*.frag"
    "${PROJECT_SOURCE_DIR}/shaders/*.vert"
    "${PROJECT_SOURCE_DIR}/shaders/*.comp"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  message(STATUS "BUILDING SHADER ${FILE_NAME}")
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    run ALL 
    DEPENDS ${SPIRV_BINARY_FILES}
)


# # uncomment below lines to print all the variables
# get_cmake_property(_variableNames VARIABLES)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()