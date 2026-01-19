#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include <algorithm>

namespace RenderStar::Common::Component
{
    ComponentModule::ComponentModule()
        : nextEntityId(0)
    {
    }

    GameObject ComponentModule::CreateEntity()
    {
        GameObject entity{ nextEntityId++ };
        entities.push_back(entity);
        return entity;
    }

    GameObject ComponentModule::CreateEntity(const std::string& name)
    {
        GameObject entity = CreateEntity();
        namePool.Add(entity, name);
        return entity;
    }

    void ComponentModule::DestroyEntity(GameObject entity)
    {
        auto iterator = std::find(entities.begin(), entities.end(), entity);

        if (iterator == entities.end())
            return;

        for (auto& [typeIndex, pool] : pools)
            pool->Remove(entity);

        namePool.Remove(entity);
        entities.erase(iterator);
    }

    bool ComponentModule::EntityExists(GameObject entity) const
    {
        return std::find(entities.begin(), entities.end(), entity) != entities.end();
    }

    std::optional<std::reference_wrapper<std::string>> ComponentModule::GetEntityName(GameObject entity)
    {
        return namePool.Get(entity);
    }

    std::optional<GameObject> ComponentModule::FindEntityByName(const std::string& name)
    {
        for (auto [entity, component] : namePool)
        {
            if (component == name)
                return entity;
        }

        return std::nullopt;
    }

    void ComponentModule::RegisterSystem(ComponentSystemFunction system)
    {
        systems.push_back(std::move(system));
    }

    void ComponentModule::RunSystems()
    {
        for (auto& system : systems)
            system(*this);
    }

    void ComponentModule::OnInitialize(Module::ModuleContext& moduleContext)
    {
        logger->info("ComponentModule initialized");
    }
}
