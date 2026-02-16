#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include <glm/glm.hpp>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <unordered_map>

namespace RenderStar::Client::Gameplay
{
    struct RemotePlayer
    {
        glm::vec3 position{0.0f};
        float yaw = -90.0f;
        float pitch = 0.0f;
    };

    class ClientPlayerModule final : public Common::Module::AbstractModule
    {
    public:

        int32_t GetLocalPlayerId() const;

        std::unordered_map<int32_t, RemotePlayer> GetRemotePlayers() const;

        void SendLocalPosition(float x, float y, float z, float yaw, float pitch);

        [[nodiscard]]
        std::vector<std::type_index> GetDependencies() const override;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        std::atomic<int32_t> localPlayerId{-1};
        std::unordered_map<int32_t, RemotePlayer> remotePlayers;
        mutable std::mutex remotePlayersMutex;
    };
}
