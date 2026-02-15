#pragma once

#include "RenderStar/Common/Component/GameObject.hpp"
#include "RenderStar/Common/Component/IComponentPool.hpp"
#include <span>
#include <vector>
#include <initializer_list>

namespace RenderStar::Common::Component
{
    class ComponentView
    {
    public:

        class Iterator
        {
        public:

            Iterator(const ComponentView& view, int32_t index)
                : view(view)
                , index(index)
            {
                AdvanceToValid();
            }

            GameObject operator*() const
            {
                return view.smallestPool->GetEntities()[index];
            }

            Iterator& operator++()
            {
                ++index;
                AdvanceToValid();
                return *this;
            }

            bool operator==(const Iterator& other) const
            {
                return index == other.index;
            }

            bool operator!=(const Iterator& other) const
            {
                return index != other.index;
            }

        private:

            void AdvanceToValid()
            {
                if (view.smallestPool == nullptr)
                    return;

                int32_t size = view.smallestPool->GetSize();

                while (index < size)
                {
                    GameObject entity = view.smallestPool->GetEntities()[index];

                    if (view.AllHave(entity))
                        return;

                    ++index;
                }
            }

            const ComponentView& view;
            int32_t index;
        };

        ComponentView()
            : smallestPool(nullptr)
        {
        }

        explicit ComponentView(std::initializer_list<IComponentPool*> poolList)
            : pools(poolList)
            , smallestPool(nullptr)
        {
            if (!pools.empty())
                smallestPool = FindSmallest();
        }

        explicit ComponentView(std::vector<IComponentPool*> poolVector)
            : pools(std::move(poolVector))
            , smallestPool(nullptr)
        {
            if (!pools.empty())
                smallestPool = FindSmallest();
        }

        bool AllHave(GameObject entity) const
        {
            for (IComponentPool* pool : pools)
            {
                if (!pool->Has(entity))
                    return false;
            }

            return true;
        }

        int32_t Count() const
        {
            int32_t count = 0;

            for (auto entity : *this)
            {
                (void)entity;
                ++count;
            }

            return count;
        }

        bool IsEmpty() const
        {
            return begin() == end();
        }

        Iterator begin() const
        {
            if (smallestPool == nullptr)
                return Iterator(*this, 0);

            return Iterator(*this, 0);
        }

        Iterator end() const
        {
            if (smallestPool == nullptr)
                return Iterator(*this, 0);

            return Iterator(*this, smallestPool->GetSize());
        }

    private:

        IComponentPool* FindSmallest() const
        {
            IComponentPool* smallest = pools[0];

            for (size_t i = 1; i < pools.size(); ++i)
            {
                if (pools[i]->GetSize() < smallest->GetSize())
                    smallest = pools[i];
            }

            return smallest;
        }

        std::vector<IComponentPool*> pools;
        IComponentPool* smallestPool;
    };
}
