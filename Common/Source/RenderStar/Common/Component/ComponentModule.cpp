#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/AbstractAffector.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include <algorithm>

namespace RenderStar::Common::Component
{
    ComponentModule::Builder& ComponentModule::Builder::Affector(std::unique_ptr<AbstractAffector> affector)
    {
        affectors.push_back(std::move(affector));
        return *this;
    }

    std::unique_ptr<ComponentModule> ComponentModule::Builder::Build()
    {
        auto module = std::make_unique<ComponentModule>();

        for (auto& affector : affectors)
            module->RegisterSubModule(std::move(affector));

        return module;
    }

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
        authorityPool.Remove(entity);
        dirtyEntities.erase(entity.id);
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

    void ComponentModule::RunAffectors()
    {
        for (auto& subModule : subModules)
        {
            if (auto* affector = dynamic_cast<AbstractAffector*>(subModule.get()))
                affector->Affect(*this);
        }
    }

    void ComponentModule::SetEntityAuthority(GameObject entity, EntityAuthority authority)
    {
        if (authorityPool.Has(entity))
        {
            auto& existing = authorityPool.Require(entity);
            existing = authority;
        }
        else
        {
            authorityPool.Add(entity, authority);
        }
    }

    EntityAuthority ComponentModule::GetEntityAuthority(GameObject entity) const
    {
        auto opt = authorityPool.Get(entity);

        if (opt.has_value())
            return opt->get();

        return EntityAuthority::Nobody();
    }

    bool ComponentModule::CheckAuthority(GameObject entity, const AuthorityContext& caller) const
    {
        auto authority = GetEntityAuthority(entity);
        return authority.CanModify(caller.level, caller.ownerId);
    }

    void ComponentModule::MarkEntityDirty(GameObject entity)
    {
        dirtyEntities.insert(entity.id);
    }

    std::unordered_set<int32_t> ComponentModule::ConsumeDirtyEntities()
    {
        auto result = std::move(dirtyEntities);
        dirtyEntities.clear();
        return result;
    }

    void ComponentModule::OnInitialize(Module::ModuleContext& moduleContext)
    {
        logger->info("ComponentModule initialized");
    }
}
