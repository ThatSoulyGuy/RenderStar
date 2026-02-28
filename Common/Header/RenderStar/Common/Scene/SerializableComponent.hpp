#pragma once

#include <type_traits>

namespace pugi
{
    class xml_node;
}

namespace RenderStar::Common::Scene
{
    class EntityIdRemapper;

    template<typename T>
    struct SerializableComponent : std::false_type {};

    template<typename T>
    concept HasRemap = requires(T& component, const EntityIdRemapper& remapper)
    {
        { SerializableComponent<T>::Remap(component, remapper) };
    };
}
