#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "RenderStar/Common/Scene/ComponentSerializer.hpp"
#include "RenderStar/Common/Scene/EntityIdRemapper.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"

namespace pugi
{
    class xml_node;
}

namespace RenderStar::Common::Scene
{
    struct ComponentSerializerEntry
    {
        std::string xmlTagName;
        std::type_index typeIndex{ typeid(void) };

        std::function<void(Component::GameObject entity, Component::ComponentModule& ecs, pugi::xml_node& entityNode)> serialize;
        std::function<void(Component::GameObject entity, Component::ComponentModule& ecs, const pugi::xml_node& componentNode)> deserialize;
        std::function<void(Component::ComponentModule& ecs, const std::unordered_set<int32_t>& ownedEntities, const EntityIdRemapper& remapper)> remapReferences;
    };

    class ComponentSerializerRegistry
    {
    public:

        template <typename ComponentType>
        void Register(const std::string& xmlTagName, ComponentSerializer<ComponentType> serializer);

        [[nodiscard]]
        const std::vector<ComponentSerializerEntry>& GetSerializers() const;

        [[nodiscard]]
        const ComponentSerializerEntry* FindByXmlTag(const std::string& tagName) const;

        [[nodiscard]]
        const ComponentSerializerEntry* FindByTypeIndex(std::type_index typeIndex) const;

    private:

        std::vector<ComponentSerializerEntry> entries;
        std::unordered_map<std::string, size_t> tagLookup;
        std::unordered_map<std::type_index, size_t> typeLookup;
    };
}

#include "RenderStar/Common/Scene/ComponentSerializerRegistry.inl"