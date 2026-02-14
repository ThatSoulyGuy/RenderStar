#include "RenderStar/Common/Scene/SceneModule.hpp"
#include "RenderStar/Common/Scene/EntityIdRemapper.hpp"
#include "RenderStar/Common/Scene/SceneEvents.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Event/IEventBus.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include <pugixml.hpp>
#include <sstream>

namespace RenderStar::Common::Scene
{
    SceneModule::SceneModule() : componentModule(nullptr), eventBus(nullptr) { }

    void SceneModule::OnInitialize(Module::ModuleContext& context)
    {
        const auto ecsOpt = context.GetModule<Component::ComponentModule>();

        if (!ecsOpt.has_value())
        {
            logger->error("ComponentModule not found, SceneModule cannot function");
            return;
        }

        componentModule = &ecsOpt.value().get();

        logger->info("SceneModule initialized");
    }

    Component::GameObject SceneModule::CreateEntity()
    {
        const auto entity = componentModule->CreateEntity();
        ownedEntities.insert(entity.id);

        return entity;
    }

    Component::GameObject SceneModule::CreateEntity(const std::string& name)
    {
        const auto entity = componentModule->CreateEntity(name);
        ownedEntities.insert(entity.id);

        return entity;
    }

    void SceneModule::DestroyEntity(const Component::GameObject entity)
    {
        ownedEntities.erase(entity.id);
        preservedComponents.erase(entity.id);
        componentModule->DestroyEntity(entity);
    }

    bool SceneModule::IsOwnedEntity(const Component::GameObject entity) const
    {
        return ownedEntities.contains(entity.id);
    }

    bool SceneModule::SaveScene(const std::string& filePath, const SceneDescriptor& descriptor)
    {
        if (eventBus)
            eventBus->PublishRaw(std::make_unique<ScenePreSaveEvent>(filePath), Event::EventPriority::NORMAL);

        pugi::xml_document document;

        auto declaration = document.prepend_child(pugi::node_declaration);
        declaration.append_attribute("version").set_value("1.0");
        declaration.append_attribute("encoding").set_value("UTF-8");

        auto root = document.append_child("Scene");
        WriteMetadata(root, descriptor);
        WriteEntities(root);

        const bool success = document.save_file(filePath.c_str(), "    ");

        if (success)
        {
            auto savedDescriptor = descriptor;
            savedDescriptor.filePath = filePath;
            currentScene = savedDescriptor;

            logger->info("Scene '{}' saved to {}", descriptor.name, filePath);
        }
        else
        {
            logger->error("Failed to save scene to {}", filePath);
        }

        if (eventBus)
            eventBus->PublishRaw(std::make_unique<ScenePostSaveEvent>(filePath, success), Event::EventPriority::NORMAL);

        return success;
    }

    bool SceneModule::LoadScene(const std::string& filePath)
    {
        if (eventBus)
            eventBus->PublishRaw(std::make_unique<ScenePreLoadEvent>(filePath), Event::EventPriority::NORMAL);

        ClearScene();

        pugi::xml_document document;
        const auto result = document.load_file(filePath.c_str());

        if (!result)
        {
            logger->error("Failed to parse scene file {}: {}", filePath, result.description());

            if (eventBus)
                eventBus->PublishRaw(std::make_unique<ScenePostLoadEvent>(SceneDescriptor{}, false), Event::EventPriority::NORMAL);

            return false;
        }

        const auto root = document.child("Scene");

        if (root.empty())
        {
            logger->error("Scene file {} has no <Scene> root element", filePath);

            if (eventBus)
                eventBus->PublishRaw(std::make_unique<ScenePostLoadEvent>(SceneDescriptor{}, false), Event::EventPriority::NORMAL);

            return false;
        }

        SceneDescriptor descriptor;
        ReadMetadata(root, descriptor);
        descriptor.filePath = filePath;

        ReadEntities(root);

        currentScene = descriptor;

        logger->info("Scene '{}' loaded from {} ({} entities)", descriptor.name, filePath, ownedEntities.size());

        if (eventBus)
            eventBus->PublishRaw(std::make_unique<ScenePostLoadEvent>(descriptor, true), Event::EventPriority::NORMAL);

        return true;
    }

    void SceneModule::ClearScene()
    {
        const auto entitiesToDestroy = ownedEntities;

        for (const auto entityId : entitiesToDestroy)
            componentModule->DestroyEntity(Component::GameObject{ entityId });

        ownedEntities.clear();
        preservedComponents.clear();
        currentScene.reset();

        logger->info("Scene cleared");

        if (eventBus)
            eventBus->PublishRaw(std::make_unique<SceneClearedEvent>(), Event::EventPriority::NORMAL);
    }

    const SceneDescriptor& SceneModule::GetCurrentSceneDescriptor() const
    {
        return currentScene.value();
    }

    bool SceneModule::HasActiveScene() const
    {
        return currentScene.has_value();
    }

    void SceneModule::SetEventBus(Event::IEventBus* bus)
    {
        eventBus = bus;
    }

    void SceneModule::WriteMetadata(pugi::xml_node& root, const SceneDescriptor& descriptor) const
    {
        root.append_attribute("name").set_value(descriptor.name.c_str());
        root.append_attribute("version").set_value(descriptor.version.c_str());
        root.append_attribute("formatVersion").set_value("1");

        if (!descriptor.author.empty())
            root.append_attribute("author").set_value(descriptor.author.c_str());

        if (!descriptor.description.empty())
        {
            auto descNode = root.append_child("Description");
            descNode.text().set(descriptor.description.c_str());
        }
    }

    void SceneModule::ReadMetadata(const pugi::xml_node& root, SceneDescriptor& descriptor) const
    {
        descriptor.name = root.attribute("name").as_string("");
        descriptor.version = root.attribute("version").as_string("1.0");
        descriptor.author = root.attribute("author").as_string("");

        if (const auto descNode = root.child("Description"); !descNode.empty())
            descriptor.description = descNode.text().as_string("");
    }

    void SceneModule::WriteEntities(pugi::xml_node& root)
    {
        auto entitiesNode = root.append_child("Entities");
        entitiesNode.append_attribute("count").set_value(static_cast<int>(ownedEntities.size()));

        const auto& serializers = registry.GetSerializers();

        for (const auto entityId : ownedEntities)
        {
            const Component::GameObject entity{ entityId };

            auto entityNode = entitiesNode.append_child("Entity");
            entityNode.append_attribute("id").set_value(entityId);

            const auto nameOpt = componentModule->GetEntityName(entity);

            if (nameOpt.has_value())
                entityNode.append_attribute("name").set_value(nameOpt.value().get().c_str());

            for (const auto& serializer : serializers)
                serializer.serialize(entity, *componentModule, entityNode);

            if (const auto it = preservedComponents.find(entityId); it != preservedComponents.end())
            {
                for (const auto& xmlFragment : it->second)
                {
                    pugi::xml_document fragmentDoc;

                    if (fragmentDoc.load_string(xmlFragment.c_str()))
                        entityNode.append_copy(fragmentDoc.document_element());
                }
            }
        }
    }

    void SceneModule::ReadEntities(const pugi::xml_node& root)
    {
        const auto entitiesNode = root.child("Entities");

        if (entitiesNode.empty())
            return;

        EntityIdRemapper remapper;

        // Pass 1: Create all entities and build ID remap table
        for (auto entityNode = entitiesNode.child("Entity"); entityNode; entityNode = entityNode.next_sibling("Entity"))
        {
            const int32_t savedId = entityNode.attribute("id").as_int(-1);
            const std::string name = entityNode.attribute("name").as_string("");

            const Component::GameObject newEntity = name.empty() ? CreateEntity() : CreateEntity(name);

            remapper.RecordMapping(savedId, newEntity);
        }

        // Pass 2: Deserialize components
        for (auto entityNode = entitiesNode.child("Entity"); entityNode; entityNode = entityNode.next_sibling("Entity"))
        {
            const int32_t savedId = entityNode.attribute("id").as_int(-1);
            const Component::GameObject entity = remapper.Remap(savedId);

            for (auto componentNode = entityNode.first_child(); componentNode; componentNode = componentNode.next_sibling())
            {
                const std::string tagName = componentNode.name();

                if (tagName == "Description")
                    continue;

                const auto* serializer = registry.FindByXmlTag(tagName);

                if (serializer != nullptr)
                {
                    serializer->deserialize(entity, *componentModule, componentNode);
                }
                else
                {
                    std::ostringstream oss;
                    componentNode.print(oss, "", pugi::format_raw);
                    preservedComponents[entity.id].push_back(oss.str());

                    logger->debug("Preserved unrecognized component '{}' on entity {}", tagName, entity.id);
                }
            }
        }

        // Pass 3: Remap entity references
        RemapEntityReferences(remapper);
    }

    void SceneModule::RemapEntityReferences(const EntityIdRemapper& remapper)
    {
        for (const auto& serializer : registry.GetSerializers())
        {
            if (serializer.remapReferences)
                serializer.remapReferences(*componentModule, ownedEntities, remapper);
        }
    }
}
