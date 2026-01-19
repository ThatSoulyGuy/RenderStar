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

        virtual bool Has(GameObject entity) const = 0;

        virtual int32_t Size() const = 0;

        virtual std::span<const GameObject> Entities() const = 0;
    };
}
