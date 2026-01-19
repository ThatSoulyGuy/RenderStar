#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace RenderStar::Common::Component
{
    struct Transform
    {
        glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

        glm::mat4 localMatrix{ 1.0f };
        glm::mat4 worldMatrix{ 1.0f };

        glm::vec3 worldPosition{ 0.0f, 0.0f, 0.0f };
        glm::quat worldRotation{ 1.0f, 0.0f, 0.0f, 0.0f };
        glm::vec3 worldScale{ 1.0f, 1.0f, 1.0f };
    };
}
