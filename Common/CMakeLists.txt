add_library(RenderStarCommon STATIC)
add_library(RenderStar::Common ALIAS RenderStarCommon)

file (GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Common/Header/RenderStar/Common/*.hpp")
file (GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Common/Source/RenderStar/Common/*.cpp")

target_sources(RenderStarCommon PRIVATE ${SOURCES} ${HEADERS})

target_include_directories(RenderStarCommon PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Header>
    $<INSTALL_INTERFACE:Header>
)

target_link_libraries(RenderStarCommon PUBLIC
    glm::glm
    spdlog::spdlog
    pugixml::pugixml
)

target_compile_definitions(RenderStarCommon PUBLIC
    GLM_FORCE_RADIANS
    GLM_FORCE_DEPTH_ZERO_TO_ONE
)
