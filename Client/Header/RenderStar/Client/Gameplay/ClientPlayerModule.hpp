#pragma once

#include "RenderStar/Common/Component/GameObject.hpp"
#include "RenderStar/Common/Module/AbstractModule.hpp"
#include "RenderStar/Common/Network/Packets/PlayerStatePacket.hpp"
#include <atomic>
#include <cstdint>
#include <deque>
#include <glm/glm.hpp>
#include <mutex>
#include <queue>

namespace RenderStar::Common::Component
{
    class ComponentModule;
}

namespace RenderStar::Common::Physics
{
    class PhysicsModule;
}

namespace RenderStar::Client::Gameplay
{
    class PlayerControllerAffector;
}

namespace RenderStar::Client::Network
{
    class ClientNetworkModule;
}

namespace RenderStar::Client::Gameplay
{
    struct PredictedInput
    {
        int32_t sequenceNumber = 0;
        uint8_t flags = 0;
        float yaw = 0.0f;
        float pitch = 0.0f;
        float deltaTime = 0.0f;
        glm::vec3 predictedPos{0.0f};
    };

    class ClientPlayerModule final : public Common::Module::AbstractModule
    {
    public:

        int32_t GetLocalPlayerId() const;

        Common::Component::GameObject GetLocalPlayerEntity() const;

        void CheckForLocalPlayerEntity(Common::Component::ComponentModule& componentModule);

        void SendInputToServer(Common::Component::ComponentModule& componentModule);

        void ProcessServerStateUpdates(Common::Component::ComponentModule& componentModule);

        void SetPlayerControllerAffector(PlayerControllerAffector* affector) { playerControllerAffector = affector; }

        [[nodiscard]]
        std::vector<std::type_index> GetDependencies() const override;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        Common::Physics::PhysicsModule* physicsModule = nullptr;
        Network::ClientNetworkModule* networkModule = nullptr;
        PlayerControllerAffector* playerControllerAffector = nullptr;

        std::atomic<int32_t> localPlayerId{-1};
        Common::Component::GameObject localPlayerEntity = Common::Component::GameObject::Invalid();
        bool localPlayerSetUp = false;

        std::deque<PredictedInput> predictionBuffer;
        int32_t nextSequenceNumber = 0;

        std::mutex stateMutex;
        std::queue<Common::Network::Packets::PlayerStatePacket> pendingStates;
    };
}
