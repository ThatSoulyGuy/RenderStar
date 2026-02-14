#pragma once

namespace RenderStar::Common::Scene
{
    template <typename ComponentType>
    void SceneModule::RegisterComponentSerializer(const std::string& xmlTagName, ComponentSerializer<ComponentType> serializer)
    {
        registry.Register<ComponentType>(xmlTagName, std::move(serializer));
        logger->info("Registered component serializer: {}", xmlTagName);
    }
}
