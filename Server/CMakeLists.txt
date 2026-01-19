add_executable(RenderStarServer)
add_executable(RenderStar::Server ALIAS RenderStarServer)

file (GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Server/Header/RenderStar/Server/*.hpp")
file (GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Server/Source/RenderStar/Server/*.cpp")

target_sources(RenderStarServer PRIVATE ${SOURCES} ${HEADERS})

target_include_directories(RenderStarServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Header
)

target_link_libraries(RenderStarServer PRIVATE
    RenderStar::Common
    asio
)

if(WIN32)
    target_link_libraries(RenderStarServer PRIVATE ws2_32 wsock32)
endif()

if(RENDERSTAR_ENABLE_VALIDATION)
    target_compile_definitions(RenderStarServer PRIVATE RENDERSTAR_ENABLE_VALIDATION)
endif()

set(ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/Resources/assets")
set(ASSETS_OUTPUT_DIR "$<TARGET_FILE_DIR:RenderStarServer>/assets")

add_custom_target(CopyServerAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${ASSETS_SOURCE_DIR}
        ${ASSETS_OUTPUT_DIR}
    COMMENT "Copying assets to server executable directory"
)

add_dependencies(RenderStarServer CopyServerAssets)
