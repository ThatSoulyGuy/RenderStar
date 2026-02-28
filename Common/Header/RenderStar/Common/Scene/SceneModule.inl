#pragma once

#include "RenderStar/Common/Scene/SerializableComponent.hpp"

namespace RenderStar::Common::Scene
{
    template <typename ComponentType>
    void SceneModule::RegisterComponentSerializer(const std::string& xmlTagName, ComponentSerializer<ComponentType> serializer)
    {
        registry.Register<ComponentType>(xmlTagName, std::move(serializer));
        logger->info("Registered component serializer: {}", xmlTagName);
    }

    template <typename ComponentType>
    void SceneModule::RegisterSerializableComponent()
    {
        static_assert(SerializableComponent<ComponentType>::value, "ComponentType must specialize SerializableComponent");

        ComponentSerializer<ComponentType> serializer;
        serializer.write = [](const ComponentType& component, pugi::xml_node& node) { SerializableComponent<ComponentType>::Write(component, node); };
        serializer.read = [](const pugi::xml_node& node) -> ComponentType { return SerializableComponent<ComponentType>::Read(node); };

        if constexpr (HasRemap<ComponentType>)
            serializer.remap = [](ComponentType& component, const EntityIdRemapper& remapper) { SerializableComponent<ComponentType>::Remap(component, remapper); };

        RegisterComponentSerializer<ComponentType>(SerializableComponent<ComponentType>::XmlTag, std::move(serializer));
    }
}
