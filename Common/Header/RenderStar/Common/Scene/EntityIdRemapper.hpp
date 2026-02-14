#pragma once

#include "RenderStar/Common/Component/GameObject.hpp"
#include <unordered_map>
#include <cstdint>

namespace RenderStar::Common::Scene
{
    class EntityIdRemapper
    {

    public:

        void RecordMapping(const int32_t savedId, const Component::GameObject newEntity)
        {
            savedToNew[savedId] = newEntity;
        }

        [[nodiscard]]
        Component::GameObject Remap(const int32_t savedId) const
        {
            if (const auto iterator = savedToNew.find(savedId); iterator != savedToNew.end())
                return iterator->second;

            return Component::GameObject{ Component::GameObject::INVALID_ID };
        }

        [[nodiscard]]
        bool HasMapping(const int32_t savedId) const
        {
            return savedToNew.contains(savedId);
        }

        [[nodiscard]]
        const std::unordered_map<int32_t, Component::GameObject>& GetAllMappings() const
        {
            return savedToNew;
        }

    private:

        std::unordered_map<int32_t, Component::GameObject> savedToNew;
    };
}
