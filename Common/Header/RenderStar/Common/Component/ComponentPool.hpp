#pragma once

#include "RenderStar/Common/Component/IComponentPool.hpp"
#include "RenderStar/Common/Component/GameObject.hpp"
#include <functional>
#include <optional>
#include <stdexcept>
#include <vector>

namespace RenderStar::Common::Component
{
    using EntityIdentifier = int32_t;
    using DenseIndex = int32_t;

    template<typename ComponentType>
    class ComponentPool final : public IComponentPool
    {
    public:

        static constexpr DenseIndex INVALID_INDEX = -1;

        using ComponentFactory = std::function<ComponentType()>;

        struct Entry
        {
            GameObject entity;
            ComponentType& component;
        };

        class Iterator
        {
        public:

            Iterator(ComponentPool& pool, const DenseIndex index) : pool(pool), index(index) { }

            Entry operator*()
            {
                return { pool.denseToEntity[index], pool.denseComponents[index] };
            }

            Iterator& operator++()
            {
                ++index;
                return *this;
            }

            bool operator!=(const Iterator& other) const
            {
                return index != other.index;
            }

        private:

            ComponentPool& pool;
            DenseIndex index;
        };

        ComponentPool() : factory([] { return ComponentType{}; }) { }

        explicit ComponentPool(ComponentFactory componentFactory) : factory(std::move(componentFactory)) { }

        ComponentType& Add(const GameObject entity)
        {
            if (Has(entity))
                return Get(entity).value().get();

            EnsureSparseCapacity(entity.id);

            ComponentType component = factory();
            const auto denseIndex = static_cast<DenseIndex>(denseComponents.size());

            denseComponents.push_back(std::move(component));
            denseToEntity.push_back(entity);
            sparseArray[entity.id] = denseIndex;

            return denseComponents.back();
        }

        ComponentType& Add(const GameObject entity, ComponentType component)
        {
            if (Has(entity))
                return Get(entity).value().get();

            EnsureSparseCapacity(entity.id);

            const auto denseIndex = static_cast<DenseIndex>(denseComponents.size());

            denseComponents.push_back(std::move(component));
            denseToEntity.push_back(entity);
            sparseArray[entity.id] = denseIndex;

            return denseComponents.back();
        }

        void Remove(GameObject entity) override
        {
            if (!Has(entity))
                return;

            DenseIndex indexToRemove = sparseArray[entity.id];
            DenseIndex lastIndex = static_cast<DenseIndex>(denseComponents.size()) - 1;

            if (indexToRemove != lastIndex)
            {
                GameObject lastEntity = denseToEntity[lastIndex];

                denseComponents[indexToRemove] = std::move(denseComponents[lastIndex]);
                denseToEntity[indexToRemove] = lastEntity;
                sparseArray[lastEntity.id] = indexToRemove;
            }

            denseComponents.pop_back();
            denseToEntity.pop_back();
            sparseArray[entity.id] = INVALID_INDEX;
        }

        std::optional<std::reference_wrapper<ComponentType>> Get(GameObject entity)
        {
            if (!Has(entity))
                return std::nullopt;

            return std::ref(denseComponents[sparseArray[entity.id]]);
        }

        std::optional<std::reference_wrapper<const ComponentType>> Get(GameObject entity) const
        {
            if (!Has(entity))
                return std::nullopt;

            return std::cref(denseComponents[sparseArray[entity.id]]);
        }

        ComponentType& Require(GameObject entity)
        {
            auto component = Get(entity);

            if (!component.has_value())
                throw std::runtime_error("Entity does not have required component");

            return component.value().get();
        }

        [[nodiscard]]
        bool Has(const GameObject entity) const override
        {
            const EntityIdentifier entityId = entity.id;

            return entityId >= 0 && entityId < static_cast<EntityIdentifier>(sparseArray.size()) && sparseArray[entityId] != INVALID_INDEX;
        }

        [[nodiscard]]
        uint32_t GetSize() const override
        {
            return static_cast<int32_t>(denseComponents.size());
        }

        [[nodiscard]]
        std::span<const GameObject> GetEntities() const override
        {
            return denseToEntity;
        }

        std::span<ComponentType> GetComponents()
        {
            return denseComponents;
        }

        std::span<const ComponentType> GetComponents() const
        {
            return denseComponents;
        }

        Iterator begin()
        {
            return Iterator(*this, 0);
        }

        Iterator end()
        {
            return Iterator(*this, static_cast<DenseIndex>(denseComponents.size()));
        }

    private:

        void EnsureSparseCapacity(const EntityIdentifier entityId)
        {
            while (static_cast<EntityIdentifier>(sparseArray.size()) <= entityId)
                sparseArray.push_back(INVALID_INDEX);
        }

        std::vector<ComponentType> denseComponents;
        std::vector<GameObject> denseToEntity;
        std::vector<DenseIndex> sparseArray;

        ComponentFactory factory;
    };
}
