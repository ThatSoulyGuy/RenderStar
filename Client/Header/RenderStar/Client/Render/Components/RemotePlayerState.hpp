#pragma once

#include <deque>
#include <glm/glm.hpp>

namespace RenderStar::Client::Render::Components
{
    struct RemotePlayerState
    {
        struct Snapshot
        {
            glm::vec3 position{0.0f};
            float yaw = 0.0f;
            float pitch = 0.0f;
            double serverTime = 0.0;
        };

        std::deque<Snapshot> snapshots;
        static constexpr size_t MAX_SNAPSHOTS = 30;
        bool initialized = false;
    };
}
