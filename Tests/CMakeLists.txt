enable_testing()

add_executable(RenderStarTests)

target_sources(RenderStarTests PRIVATE
    Source/ComponentPoolTest.cpp
    Source/PacketBufferTest.cpp
    Source/EventBusTest.cpp
    Source/GameObjectTest.cpp
    Source/TypeNameTest.cpp
    Source/NetworkingTest.cpp
)

target_include_directories(RenderStarTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Header
)

target_link_libraries(RenderStarTests PRIVATE
    RenderStar::Common
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_add_tests(TARGET RenderStarTests)
