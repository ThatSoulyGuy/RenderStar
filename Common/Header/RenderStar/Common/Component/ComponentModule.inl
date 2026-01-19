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
    void ComponentModule::RemoveComponent(GameObject entity)
    {
        auto typeIndex = std::type_index(typeid(ComponentType));
        auto iterator = pools.find(typeIndex);

        if (iterator != pools.end())
            iterator->second->Remove(entity);
    }

    template<typename ComponentType>
    std::optional<std::reference_wrapper<ComponentType>> ComponentModule::GetComponent(GameObject entity)
    {
        auto typeIndex = std::type_index(typeid(ComponentType));
        auto iterator = pools.find(typeIndex);

        if (iterator == pools.end())
            return std::nullopt;

        auto& pool = static_cast<ComponentPool<ComponentType>&>(*iterator->second);
        return pool.Get(entity);
    }

    template<typename ComponentType>
    bool ComponentModule::HasComponent(GameObject entity) const
    {
        auto typeIndex = std::type_index(typeid(ComponentType));
        auto iterator = pools.find(typeIndex);

        if (iterator == pools.end())
            return false;

        return iterator->second->Has(entity);
    }

    template<typename ComponentType>
    ComponentPool<ComponentType>& ComponentModule::GetPool()
    {
        EnsurePoolExists<ComponentType>();
        auto typeIndex = std::type_index(typeid(ComponentType));
        return static_cast<ComponentPool<ComponentType>&>(*pools[typeIndex]);
    }

    template<typename ComponentType>
    void ComponentModule::EnsurePoolExists()
    {
        auto typeIndex = std::type_index(typeid(ComponentType));

        if (!pools.contains(typeIndex))
            pools[typeIndex] = std::make_unique<ComponentPool<ComponentType>>();
    }
}
