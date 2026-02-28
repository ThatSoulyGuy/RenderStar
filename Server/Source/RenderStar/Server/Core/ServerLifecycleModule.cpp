#include "RenderStar/Server/Core/ServerLifecycleModule.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/MapGeometry.hpp"
#include "RenderStar/Common/Component/EntityAuthority.hpp"
#include "RenderStar/Common/Configuration/ConfigurationModule.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Scene/SceneModule.hpp"
#include "RenderStar/Server/Core/ServerSceneModule.hpp"

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
        }

        logger->info("ServerLifecycleModule initialized");
    }

    std::vector<std::type_index> ServerLifecycleModule::GetDependencies() const
    {
        return DependsOn<
            Common::Scene::SceneModule,
            Common::Component::ComponentModule,
            Common::Configuration::ConfigurationModule,
            ServerSceneModule>();
    }
}
