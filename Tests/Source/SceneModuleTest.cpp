#include <gtest/gtest.h>
#include "RenderStar/Common/Scene/SceneModule.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Common/Component/Components/Hierarchy.hpp"
#include "RenderStar/Common/Module/ModuleManager.hpp"
#include <filesystem>
#include <fstream>

using namespace RenderStar::Common::Scene;
using namespace RenderStar::Common::Component;
using namespace RenderStar::Common::Module;

class SceneModuleTest : public ::testing::Test
{
protected:
    std::unique_ptr<ModuleManager> manager;
    ComponentModule* ecs = nullptr;
    SceneModule* scene = nullptr;
    std::string tempFilePath;

    void SetUp() override
    {
        auto builder = ModuleManager::Builder();
        auto cm = std::make_unique<ComponentModule>();
        auto sm = std::make_unique<SceneModule>();
        ecs = cm.get();
        scene = sm.get();
        manager = builder.Module(std::move(cm)).Module(std::move(sm)).Build();
        manager->Start();

        RegisterDefaultSerializers();

        tempFilePath = (std::filesystem::temp_directory_path() / "test_scene.xml").string();
    }

    void TearDown() override
    {
        manager->Shutdown();
        if (std::filesystem::exists(tempFilePath))
            std::filesystem::remove(tempFilePath);
    }

    void RegisterDefaultSerializers()
    {
        scene->RegisterComponentSerializer<Transform>("Transform", ComponentSerializer<Transform>{
            [](const Transform& t, pugi::xml_node& node)
            {
                node.append_attribute("px").set_value(t.position.x);
                node.append_attribute("py").set_value(t.position.y);
                node.append_attribute("pz").set_value(t.position.z);
                node.append_attribute("sx").set_value(t.scale.x);
                node.append_attribute("sy").set_value(t.scale.y);
                node.append_attribute("sz").set_value(t.scale.z);
            },
            [](const pugi::xml_node& node) -> Transform
            {
                Transform t;
                t.position.x = node.attribute("px").as_float();
                t.position.y = node.attribute("py").as_float();
                t.position.z = node.attribute("pz").as_float();
                t.scale.x = node.attribute("sx").as_float(1.0f);
                t.scale.y = node.attribute("sy").as_float(1.0f);
                t.scale.z = node.attribute("sz").as_float(1.0f);
                return t;
            },
            nullptr
        });
    }
};

TEST_F(SceneModuleTest, CreateOwnedEntity)
{
    auto entity = scene->CreateEntity();
    EXPECT_TRUE(entity.IsValid());
    EXPECT_TRUE(scene->IsOwnedEntity(entity));
}

TEST_F(SceneModuleTest, CreateNamedOwnedEntity)
{
    auto entity = scene->CreateEntity("TestEntity");
    EXPECT_TRUE(scene->IsOwnedEntity(entity));
    auto name = ecs->GetEntityName(entity);
    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(name->get(), "TestEntity");
}

TEST_F(SceneModuleTest, DestroyOwnedEntity)
{
    auto entity = scene->CreateEntity();
    scene->DestroyEntity(entity);
    EXPECT_FALSE(scene->IsOwnedEntity(entity));
    EXPECT_FALSE(ecs->EntityExists(entity));
}

TEST_F(SceneModuleTest, NoActiveSceneInitially)
{
    EXPECT_FALSE(scene->HasActiveScene());
}

TEST_F(SceneModuleTest, SaveAndLoad)
{
    auto entity = scene->CreateEntity("SaveTest");
    ecs->AddComponent<Transform>(entity, Transform{{1.0f, 2.0f, 3.0f}});

    SceneDescriptor descriptor;
    descriptor.name = "TestScene";
    descriptor.version = "1.0";
    descriptor.author = "Test";

    EXPECT_TRUE(scene->SaveScene(tempFilePath, descriptor));
    EXPECT_TRUE(scene->HasActiveScene());

    scene->ClearScene();
    EXPECT_FALSE(scene->HasActiveScene());

    EXPECT_TRUE(scene->LoadScene(tempFilePath));
    EXPECT_TRUE(scene->HasActiveScene());
}

TEST_F(SceneModuleTest, SceneDescriptorMetadata)
{
    auto entity = scene->CreateEntity();

    SceneDescriptor descriptor;
    descriptor.name = "MyScene";
    descriptor.version = "2.0";
    descriptor.author = "Author";
    descriptor.description = "A test scene";

    scene->SaveScene(tempFilePath, descriptor);

    auto& loaded = scene->GetCurrentSceneDescriptor();
    EXPECT_EQ(loaded.name, "MyScene");
}

TEST_F(SceneModuleTest, EmptySceneSaveLoad)
{
    SceneDescriptor descriptor;
    descriptor.name = "EmptyScene";

    EXPECT_TRUE(scene->SaveScene(tempFilePath, descriptor));

    scene->ClearScene();
    EXPECT_TRUE(scene->LoadScene(tempFilePath));
}

TEST_F(SceneModuleTest, ClearScene)
{
    auto entity = scene->CreateEntity();
    scene->ClearScene();
    EXPECT_FALSE(scene->IsOwnedEntity(entity));
    EXPECT_FALSE(scene->HasActiveScene());
}

TEST_F(SceneModuleTest, NonOwnedEntityNotTracked)
{
    auto entity = ecs->CreateEntity();
    EXPECT_FALSE(scene->IsOwnedEntity(entity));
}
