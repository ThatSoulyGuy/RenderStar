#pragma once

namespace RenderStar::Common::Component
{
    template<typename ComponentType>
    ComponentType& ComponentModule::AddComponent(GameObject entity)
    {
        EnsurePoolExists<ComponentType>();
        return GetPool<ComponentType>().Add(entity);
    }

    template<typename ComponentType>
    ComponentType& ComponentModule::AddComponent(GameObject entity, ComponentType component)
    {
        EnsurePoolExists<ComponentType>();
        return GetPool<ComponentType>().Add(entity, std::move(component));
    }

    template<typename ComponentType>
    void ComponentModule::RemoveComponent(const GameObject entity)
    {
        const auto typeIndex = std::type_index(typeid(ComponentType));

        if (const auto iterator = pools.find(typeIndex); iterator != pools.end())
            iterator->second->Remove(entity);
    }

    template<typename ComponentType>
    std::optional<std::reference_wrapper<ComponentType>> ComponentModule::GetComponent(GameObject entity)
    {
        const auto typeIndex = std::type_index(typeid(ComponentType));
        const auto iterator = pools.find(typeIndex);

        if (iterator == pools.end())
            return std::nullopt;

        auto& pool = static_cast<ComponentPool<ComponentType>&>(*iterator->second);
        return pool.Get(entity);
    }

    template <typename ComponentType>
    bool ComponentModule::HasComponent(GameObject entity) const
    {
        const auto typeIndex = std::type_index(typeid(ComponentType));
        const auto iterator = pools.find(typeIndex);

        if (iterator == pools.end())
            return false;

        return iterator->second->Has(entity);
    }

    template<typename ComponentType>
    ComponentPool<ComponentType>& ComponentModule::GetPool()
    {
        EnsurePoolExists<ComponentType>();

        const auto typeIndex = std::type_index(typeid(ComponentType));

        return static_cast<ComponentPool<ComponentType>&>(*pools[typeIndex]);
    }

    template<typename ComponentType>
    std::optional<std::reference_wrapper<ComponentType>> ComponentModule::GetComponentAuthorized(GameObject entity, const AuthorityContext& caller)
    {
        if (!CheckAuthority(entity, caller))
            return std::nullopt;

        auto result = GetComponent<ComponentType>(entity);

        if (result.has_value())
            MarkEntityDirty(entity);

        return result;
    }

    template<typename ComponentType>
    ComponentType& ComponentModule::AddComponentAuthorized(GameObject entity, const AuthorityContext& caller)
    {
        if (!CheckAuthority(entity, caller))
        {
            static ComponentType fallback{};
            return fallback;
        }

        MarkEntityDirty(entity);
        return AddComponent<ComponentType>(entity);
    }

    template<typename ComponentType>
    ComponentType& ComponentModule::AddComponentAuthorized(GameObject entity, ComponentType component, const AuthorityContext& caller)
    {
        if (!CheckAuthority(entity, caller))
        {
            static ComponentType fallback{};
            return fallback;
        }

        MarkEntityDirty(entity);
        return AddComponent<ComponentType>(entity, std::move(component));
    }

    template<typename ComponentType>
    void ComponentModule::RemoveComponentAuthorized(GameObject entity, const AuthorityContext& caller)
    {
        if (!CheckAuthority(entity, caller))
            return;

        MarkEntityDirty(entity);
        RemoveComponent<ComponentType>(entity);
    }

    template<typename ComponentType>
    void ComponentModule::EnsurePoolExists()
    {
        if (const auto typeIndex = std::type_index(typeid(ComponentType)); !pools.contains(typeIndex))
            pools[typeIndex] = std::make_unique<ComponentPool<ComponentType>>();
    }
}
