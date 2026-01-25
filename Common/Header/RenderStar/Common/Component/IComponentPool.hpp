#pragma once

#include "RenderStar/Common/Component/GameObject.hpp"
#include <span>
#include <cstdint>

namespace RenderStar::Common::Component
{
    class IComponentPool
    {

    public:

        virtual ~IComponentPool() = default;

        virtual void Remove(GameObject entity) = 0;

        [[nodiscard]]
        virtual bool Has(GameObject entity) const = 0;

        [[nodiscard]]
        virtual uint32_t GetSize() const = 0;

        [[nodiscard]]
        virtual std::span<const GameObject> GetEntities() const = 0;
    };
}
