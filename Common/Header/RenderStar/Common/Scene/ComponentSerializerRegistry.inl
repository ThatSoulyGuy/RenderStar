#pragma once

#include <pugixml.hpp>

namespace RenderStar::Common::Scene
{
    template <typename ComponentType>
    void ComponentSerializerRegistry::Register(const std::string& xmlTagName, ComponentSerializer<ComponentType> serializer)
    {
        ComponentSerializerEntry entry;

        entry.xmlTagName = xmlTagName;
        entry.typeIndex = std::type_index(typeid(ComponentType));

        const auto& capturedTag = xmlTagName;

        auto capturedWrite = serializer.write;
        auto capturedRead = serializer.read;
        auto capturedRemap = serializer.remap;

        entry.serialize = [capturedTag, capturedWrite](const Component::GameObject entity, Component::ComponentModule& ecs, pugi::xml_node& entityNode)
        {
            auto componentOpt = ecs.GetComponent<ComponentType>(entity);

            if (!componentOpt.has_value())
                return;

            pugi::xml_node componentNode = entityNode.append_child(capturedTag.c_str());
            capturedWrite(componentOpt.value().get(), componentNode);
        };

        entry.deserialize = [capturedRead](const Component::GameObject entity, Component::ComponentModule& ecs, const pugi::xml_node& componentNode)
        {
            ComponentType component = capturedRead(componentNode);
            ecs.AddComponent<ComponentType>(entity, std::move(component));
        };

        if (capturedRemap)
        {
            entry.remapReferences = [capturedRemap](Component::ComponentModule& ecs, const std::unordered_set<int32_t>& ownedEntities, const EntityIdRemapper& remapper)
            {
                for (auto& pool = ecs.GetPool<ComponentType>(); auto [entity, component] : pool)
                {
                    if (ownedEntities.contains(entity.id))
                        capturedRemap(component, remapper);
                }
            };
        }

        const size_t index = entries.size();

        entries.push_back(std::move(entry));
        tagLookup[xmlTagName] = index;
        typeLookup[entry.typeIndex] = index;
    }
}
