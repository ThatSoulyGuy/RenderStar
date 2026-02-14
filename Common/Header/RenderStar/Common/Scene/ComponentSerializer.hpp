#pragma once

#include <functional>

namespace pugi
{
    class xml_node;
}

namespace RenderStar::Common::Scene
{
    class EntityIdRemapper;

    template <typename ComponentType>
    struct ComponentSerializer
    {
        std::function<void(const ComponentType& component, pugi::xml_node& node)> write;
        std::function<ComponentType(const pugi::xml_node& node)> read;
        std::function<void(ComponentType& component, const EntityIdRemapper& remapper)> remap;
    };
}
