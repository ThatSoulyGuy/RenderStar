#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Common/Scene/SceneDescriptor.hpp"
#include "RenderStar/Common/Scene/ComponentSerializerRegistry.hpp"
#include <optional>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace pugi
{
    class xml_node;
}

namespace RenderStar::Common::Event
{
    class IEventBus;
}

namespace RenderStar::Common::Component
{
    class ComponentModule;
}

namespace RenderStar::Common::Scene
{
    class EntityIdRemapper;

    class SceneModule final : public Module::AbstractModule
    {
    public:

        SceneModule();

        Component::GameObject CreateEntity();
        Component::GameObject CreateEntity(const std::string& name);
        void DestroyEntity(Component::GameObject entity);

        [[nodiscard]]
        bool IsOwnedEntity(Component::GameObject entity) const;

        template <typename ComponentType>
        void RegisterComponentSerializer(const std::string& xmlTagName, ComponentSerializer<ComponentType> serializer);

        bool SaveScene(const std::string& filePath, const SceneDescriptor& descriptor);
        bool LoadScene(const std::string& filePath);
        void ClearScene();

        [[nodiscard]]
        const SceneDescriptor& GetCurrentSceneDescriptor() const;

        [[nodiscard]]
        bool HasActiveScene() const;

        void SetEventBus(Event::IEventBus* bus);

    protected:

        void OnInitialize(Module::ModuleContext& context) override;

    private:

        void WriteMetadata(pugi::xml_node& root, const SceneDescriptor& descriptor) const;
        void ReadMetadata(const pugi::xml_node& root, SceneDescriptor& descriptor) const;
        void WriteEntities(pugi::xml_node& root);
        void ReadEntities(const pugi::xml_node& root);
        void RemapEntityReferences(const EntityIdRemapper& remapper);

        Component::ComponentModule* componentModule;
        Event::IEventBus* eventBus;
        std::optional<SceneDescriptor> currentScene;
        ComponentSerializerRegistry registry;
        std::unordered_set<int32_t> ownedEntities;
        std::unordered_map<int32_t, std::vector<std::string>> preservedComponents;
    };
}

#include "RenderStar/Common/Scene/SceneModule.inl"
