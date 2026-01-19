add_executable(RenderStarClient)
add_executable(RenderStar::Client ALIAS RenderStarClient)

file (GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Client/Header/RenderStar/Client/*.hpp")
file (GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Client/Source/RenderStar/Client/*.cpp")

target_sources(RenderStarClient PRIVATE ${SOURCES} ${HEADERS})

target_include_directories(RenderStarClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Header
)

target_link_libraries(RenderStarClient PRIVATE
    RenderStar::Common
    glfw
    glad_gl46
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator
    asio
)

if(WIN32)
    target_link_libraries(RenderStarClient PRIVATE opengl32 ws2_32 wsock32)
endif()

if(RENDERSTAR_ENABLE_VALIDATION)
    target_compile_definitions(RenderStarClient PRIVATE RENDERSTAR_ENABLE_VALIDATION)
endif()

set(ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/Resources/assets")
set(ASSETS_OUTPUT_DIR "$<TARGET_FILE_DIR:RenderStarClient>/assets")

add_custom_target(CopyClientAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${ASSETS_SOURCE_DIR}
        ${ASSETS_OUTPUT_DIR}
    COMMENT "Copying assets to executable directory"
)

add_dependencies(RenderStarClient CopyClientAssets)
