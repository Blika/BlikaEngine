include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
cmake_minimum_required(VERSION 3.11.0)
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc.exe)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++.exe)
set(CMAKE_CXX_STANDARD 17)
project(BlikaEngine)

set(GLFW_PATH ${PROJECT_SOURCE_DIR}/libs/glfw-3.3.8.bin.WIN64)
set(GLM_PATH ${PROJECT_SOURCE_DIR}/libs/glm)
set(STB_PATH ${PROJECT_SOURCE_DIR}/libs/stb)
set(TINYOBJ_PATH ${PROJECT_SOURCE_DIR}/libs/tinyobjectloader)

set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib")
set(GLFW_INCLUDE_DIRS "${GLFW_PATH}/include")
set(GLFW_LIB "${GLFW_PATH}/lib-mingw-w64")

include_directories(${GLFW_INCLUDE_DIRS} ${GLM_PATH} ${Vulkan_INCLUDE_DIRS} ${STB_PATH} ${TINYOBJ_PATH} ${PROJECT_SOURCE_DIR}/src)
link_directories(${GLFW_INCLUDE_DIRS} ${GLM_PATH} ${Vulkan_INCLUDE_DIRS} ${STB_PATH} ${TINYOBJ_PATH} ${PROJECT_SOURCE_DIR}/src)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
 
target_link_libraries(${PROJECT_NAME} ${GLFW_LIB}/libglfw3.a ${Vulkan_LIBRARIES}/vulkan-1.lib)

############## Build SHADERS #######################
 
# Find all vertex and fragment sources within shaders directory
# taken from VBlancos vulkan tutorial
# https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters/CMakeLists.txt
find_program(GLSL_VALIDATOR glslangValidator HINTS ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} /usr/bin /usr/local/bin ${VULKAN_SDK_PATH}/Bin ${VULKAN_SDK_PATH}/Bin32 $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)
 
# get all .vert and .frag files in shaders directory
file(GLOB_RECURSE GLSL_SOURCE_FILES "${PROJECT_SOURCE_DIR}/shaders/*.frag" "${PROJECT_SOURCE_DIR}/shaders/*.vert")
 
foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
  add_custom_command(OUTPUT ${SPIRV} COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV} DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)
 
add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})