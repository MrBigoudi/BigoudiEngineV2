cmake_minimum_required(VERSION 3.15)

project(BigoudiEngine)

# create the library for the renderer of the engine
add_library(BigoudiEngine STATIC ${BE_SOURCE_FILES})

# pass the variable to the source code as a preprocessor definition
add_compile_definitions(ENGINE_NAME="${PROJECT_NAME}")
add_compile_definitions(ENGINE_VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
add_compile_definitions(ENGINE_VERSION_MINOR=${PROJECT_VERSION_MINOR})
add_compile_definitions(ENGINE_DIRECTORY_PATH="${CMAKE_CURRENT_SOURCE_DIR}")

# create compile command
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# export compile command
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# dependencies
find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(OpenMP REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)

# add source files
add_subdirectory(src)

# c++ standard
target_compile_features(BigoudiEngine PUBLIC cxx_std_20)

# compile flags
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
set(BigoudiEngine_cflags BigoudiEngine_compiler_flags)

add_library(BigoudiEngine_cflags INTERFACE)
target_compile_features(BigoudiEngine_cflags INTERFACE cxx_std_20)
target_compile_options(BigoudiEngine_cflags INTERFACE
    "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
    "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>"
    ${OpenMP_CXX_FLAGS}
)
target_link_libraries(BigoudiEngine PRIVATE glfw Vulkan::Vulkan BigoudiEngine_cflags OpenMP::OpenMP_CXX)

# compile shaders
add_custom_target(
    BigoudiEngine_compile_shaders
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/compileShaders.sh
    COMMENT "Compiling engine default shaders..."
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    VERBATIM
)

add_dependencies(BigoudiEngine BigoudiEngine_compile_shaders)