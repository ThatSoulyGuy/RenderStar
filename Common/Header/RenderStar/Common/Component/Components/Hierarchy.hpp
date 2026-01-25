#pragma once

#include "RenderStar/Common/Component/GameObject.hpp"
#include <vector>

namespace RenderStar::Common::Component
{
    struct Hierarchy
    {
        GameObject parent{ GameObject::INVALID_ID };
        std::vector<GameObject> children;

        [[nodiscard]]
        bool HasParent() const
        {
            return parent.IsValid();
        }
    };
}
