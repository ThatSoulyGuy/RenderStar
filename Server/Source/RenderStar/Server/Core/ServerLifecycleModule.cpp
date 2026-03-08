#include "RenderStar/Server/Core/ServerLifecycleModule.hpp"
#include "RenderStar/Common/Asset/AssetModule.hpp"
#include "RenderStar/Common/Asset/AssetLocation.hpp"
#include "RenderStar/Common/Asset/IBinaryAsset.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/MapGeometry.hpp"
#include "RenderStar/Common/Component/EntityAuthority.hpp"
#include "RenderStar/Common/Configuration/ConfigurationModule.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Physics/PhysicsModule.hpp"
#include "RenderStar/Common/Scene/MapbinLoader.hpp"
#include "RenderStar/Common/Scene/SceneModule.hpp"
#include "RenderStar/Common/Time/TimeModule.hpp"
#include "RenderStar/Server/Core/ServerSceneModule.hpp"
#include "RenderStar/Server/Event/Buses/ServerCoreEventBus.hpp"
#include "RenderStar/Server/Physics/ServerPhysicsModule.hpp"

namespace RenderStar::Server::Core
{
    void ServerLifecycleModule::OnInitialize(Common::Module::ModuleContext& context)
    {
        auto& sceneModule = context.GetDependency<Common::Scene::SceneModule>();
        auto& componentModule = context.GetDependency<Common::Component::ComponentModule>();
        auto& configModule = context.GetDependency<Common::Configuration::ConfigurationModule>();

        std::string sceneFile;

        if (auto configOpt = configModule.For<ServerSceneModule>("render_star", "server_settings.xml"))
        {
            if (auto sceneOpt = (*configOpt)->GetString("scene_file"))
                sceneFile = *sceneOpt;
        }

        auto& assetModule = context.GetDependency<Common::Asset::AssetModule>();
        auto& physicsModule = context.GetDependency<Common::Physics::PhysicsModule>();
        auto* serverPhysicsModule = &context.GetDependency<Physics::ServerPhysicsModule>();

        if (sceneFile.empty())
        {
            logger->info("No scene_file configured, skipping scene setup");
        }
        else
        {
            auto mapEntity = sceneModule.CreateEntity("MapRoot");
            componentModule.AddComponent<Common::Component::MapGeometry>(mapEntity, Common::Component::MapGeometry{ sceneFile });
            componentModule.SetEntityAuthority(mapEntity, Common::Component::EntityAuthority::Server());

            logger->info("Created MapRoot entity with asset path: {}", sceneFile);

            // Load mapbin for collision meshes and spawn points
            auto binaryAsset = assetModule.LoadBinary(Common::Asset::AssetLocation::Parse(sceneFile));

            if (binaryAsset.IsValid())
            {
                auto scene = Common::Scene::MapbinLoader::Load(binaryAsset.Get()->GetDataView());

                if (scene.has_value())
                {
                    constexpr float mapScale = 0.1f;

                    // Create collision meshes from geometry groups
                    for (const auto& group : scene->groups)
                    {
                        int32_t vertexCount = group.vertexCount;
                        int32_t indexCount = static_cast<int32_t>(group.indices.size());

                        if (vertexCount > 0 && indexCount > 0)
                        {
                            // vertexData has 8 floats per vertex (pos x3, normal x3, uv x2), stride = 8
                            physicsModule.CreateStaticTriangleMesh(
                                group.vertexData.data(), vertexCount, 8,
                                group.indices.data(), indexCount, mapScale);
                        }
                    }

                    logger->info("Created {} collision meshes from mapbin", scene->groups.size());

                    // Extract PLAYER_START spawn points
                    for (const auto& obj : scene->gameObjects)
                    {
                        if (obj.type == Common::Scene::GameObjectType::PLAYER_START)
                        {
                            glm::vec3 spawnPos = glm::vec3(obj.posX, obj.posY, obj.posZ) * mapScale;
                            serverPhysicsModule->AddSpawnPoint(spawnPos, obj.rotY);
                        }
                    }
                }
                else
                {
                    logger->error("Failed to parse mapbin: {}", sceneFile);
                }
            }
            else
            {
                logger->warn("Failed to load mapbin asset for collision: {}", sceneFile);
            }
        }

        auto* timeModule = &context.GetDependency<Common::Time::TimeModule>();

        auto eventBus = context.GetEventBus<Event::Buses::ServerCoreEventBus>();

        if (eventBus.has_value())
        {
            eventBus->get().SetTickHandler([timeModule, serverPhysicsModule]
            {
                timeModule->Tick();
                serverPhysicsModule->Tick();
            });

            logger->info("Server tick handler set up");
        }

        logger->info("ServerLifecycleModule initialized");
    }

    std::vector<std::type_index> ServerLifecycleModule::GetDependencies() const
    {
        return DependsOn<
            Common::Scene::SceneModule,
            Common::Component::ComponentModule,
            Common::Configuration::ConfigurationModule,
            Common::Asset::AssetModule,
            Common::Physics::PhysicsModule,
            Common::Time::TimeModule,
            ServerSceneModule,
            Physics::ServerPhysicsModule>();
    }
}
