#pragma once

#include <glm/glm.hpp>

namespace RenderStar::Client::Gameplay
{
    struct PlayerController
    {
        float lookSensitivity{ 0.15f };
        float yaw{ -90.0f };
        float pitch{ 0.0f };

        glm::vec2 currentVelocity{ 0.0f };  // XZ velocity

        // Head bob state (client-only visual)
        float bobTimer = 0.0f;
        float bobAmount = 0.0f;
    };
}
