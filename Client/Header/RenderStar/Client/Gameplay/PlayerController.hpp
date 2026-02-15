#pragma once

namespace RenderStar::Client::Gameplay
{
    struct PlayerController
    {
        float moveSpeed{ 5.0f };
        float lookSensitivity{ 0.15f };
        float yaw{ -90.0f };
        float pitch{ 0.0f };
    };
}
