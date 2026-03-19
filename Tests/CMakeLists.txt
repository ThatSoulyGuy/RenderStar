enable_testing()

add_executable(RenderStarTests)

target_sources(RenderStarTests PRIVATE
    Source/GameObjectTest.cpp
    Source/ComponentPoolTest.cpp
    Source/ComponentViewTest.cpp
    Source/ComponentModuleTest.cpp
    Source/AbstractAffectorTest.cpp
    Source/EventBusTest.cpp
    Source/EventResultTest.cpp
    Source/ModuleManagerTest.cpp
    Source/AssetLocationTest.cpp
    Source/AssetCacheTest.cpp
    Source/AssetHandleTest.cpp
    Source/TextAssetTest.cpp
    Source/BinaryAssetTest.cpp
    Source/TimeModuleTest.cpp
    Source/PacketBufferTest.cpp
    Source/PacketModuleTest.cpp
    Source/SceneModuleTest.cpp
    Source/EntityIdRemapperTest.cpp
    Source/ComponentSerializerRegistryTest.cpp
    Source/TypeNameTest.cpp
    Source/ResourcePathTest.cpp
    Source/ConfigurationTest.cpp
    Source/HierarchyTest.cpp
    Source/TransformTest.cpp
    Source/RsslBackendEmitterTest.cpp
    Source/VertexTest.cpp
    Source/VertexLayoutTest.cpp
    Source/StandardUniformsTest.cpp
    Source/UniformLayoutTest.cpp
    Source/CameraTest.cpp
    Source/PlayerControllerTest.cpp
    Source/BackendCapabilitiesTest.cpp
    Source/KeyModifierTest.cpp
    Source/ClientEventsTest.cpp
    Source/ConnectionStateTest.cpp
    Source/TextureDescriptionTest.cpp
    Source/MapbinLoaderTest.cpp
    Source/MapbinLoaderV3Test.cpp
    Source/LitVertexTest.cpp
    Source/SceneDataPacketTest.cpp
    Source/EntityAuthorityTest.cpp
    Source/ComponentModuleAuthorityTest.cpp
    Source/ComponentUpdatePacketTest.cpp
    Source/AuthorityChangePacketTest.cpp
    Source/RsslCompilerTest.cpp
    Source/RsslCompilerEdgeCaseTest.cpp
    Source/StageExecutionContextTest.cpp
    Source/PipelineStageTest.cpp
    Source/RenderTargetDescriptionTest.cpp
    Source/LightComponentTest.cpp
    Source/SceneLightingDataTest.cpp
    Source/MapbinLoaderV5Test.cpp
    Source/MaterialPropertiesTest.cpp
    ${CMAKE_SOURCE_DIR}/Client/Source/RenderStar/Client/Render/Resource/Vertex.cpp
    ${CMAKE_SOURCE_DIR}/Client/Source/RenderStar/Client/Render/Platform/StageExecutionContext.cpp
    ${CMAKE_SOURCE_DIR}/Client/Source/RenderStar/Client/Render/Platform/GeometryStage.cpp
    ${CMAKE_SOURCE_DIR}/Client/Source/RenderStar/Client/Render/Platform/FullscreenStage.cpp
    ${CMAKE_SOURCE_DIR}/Client/Source/RenderStar/Client/Render/Platform/ComputeStage.cpp
    ${CMAKE_SOURCE_DIR}/Client/Source/RenderStar/Client/Render/Shader/RsslCompiler.cpp
    ${CMAKE_SOURCE_DIR}/Client/Source/RenderStar/Client/Render/Components/Camera.cpp
    ${CMAKE_SOURCE_DIR}/Client/Source/RenderStar/Client/Render/Framework/LitVertex.cpp
    ${CMAKE_SOURCE_DIR}/Client/Source/RenderStar/Client/Render/Components/Light.cpp
)

target_include_directories(RenderStarTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Header
    ${CMAKE_SOURCE_DIR}/Client/Header
)

target_link_libraries(RenderStarTests PRIVATE
    RenderStar::Common
    GTest::gtest
    GTest::gtest_main
)

include(GoogleTest)
gtest_add_tests(TARGET RenderStarTests)
