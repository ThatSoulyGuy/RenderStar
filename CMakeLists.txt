cmake_minimum_required(VERSION 3.25)
project(RenderStar VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(RENDERSTAR_BUILD_TESTS "Build tests" ON)
option(RENDERSTAR_BUILD_CLIENT "Build client" ON)
option(RENDERSTAR_BUILD_SERVER "Build server" ON)
option(RENDERSTAR_ENABLE_VALIDATION "Enable Vulkan validation layers" OFF)
option(RENDERSTAR_ENABLE_ASAN "Enable AddressSanitizer" OFF)

if(RENDERSTAR_ENABLE_ASAN)
    if(MSVC)
        add_compile_options(/fsanitize=address)
    else()
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()
endif()

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    add_link_options(-static-libgcc -static-libstdc++)
endif()

include(CMake/Dependencies.cmake)

add_subdirectory(Common)

if(RENDERSTAR_BUILD_CLIENT)
    add_subdirectory(Client)
endif()

if(RENDERSTAR_BUILD_SERVER)
    add_subdirectory(Server)
endif()

if(RENDERSTAR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()
