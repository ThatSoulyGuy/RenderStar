#pragma once

#include "RenderStar/Common/Component/GameObject.hpp"
#include "RenderStar/Common/Module/AbstractModule.hpp"
#include <cstdint>
#include <glm/glm.hpp>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

class btKinematicCharacterController;
class btPairCachingGhostObject;

namespace RenderStar::Common::Component
{
    class ComponentModule;
}

namespace RenderStar::Common::Physics
{
    class PhysicsModule;
}

namespace RenderStar::Common::Time
{
    class TimeModule;
}

namespace RenderStar::Server::Network
{
    class ServerNetworkModule;
    class ClientConnection;
}

namespace RenderStar::Server::Physics
{
    struct QueuedInput
    {
        int32_t sequenceNumber = 0;
        uint8_t flags = 0;
        float yaw = 0.0f;
        float pitch = 0.0f;
        float deltaTime = 0.0f;
    };

    struct ServerPlayerPhysicsState
    {
        int32_t playerId = -1;
        Common::Component::GameObject entity;
        btKinematicCharacterController* controller = nullptr;
        btPairCachingGhostObject* ghostObject = nullptr;
        std::shared_ptr<Network::ClientConnection> connection;

        std::vector<QueuedInput> inputQueue;
        int32_t lastProcessedSequence = 0;
        float currentYaw = 0.0f;
        float currentPitch = 0.0f;
        float currentVelocityX = 0.0f;
        float currentVelocityZ = 0.0f;
    };

    class ServerPhysicsModule final : public Common::Module::AbstractModule
    {
    public:

        void OnPlayerJoined(int32_t playerId, Common::Component::GameObject entity,
                            std::shared_ptr<Network::ClientConnection> connection,
                            const glm::vec3& spawnPos);

        void OnPlayerLeft(int32_t playerId);

        void QueueInput(int32_t playerId, int32_t sequenceNumber, uint8_t flags,
                        float yaw, float pitch, float deltaTime);

        void Tick();

        void AddSpawnPoint(const glm::vec3& position, float yaw);

        glm::vec3 GetSpawnPosition() const;
        float GetSpawnYaw() const;

        [[nodiscard]]
        std::vector<std::type_index> GetDependencies() const override;

    protected:

        void OnInitialize(Common::Module::ModuleContext& context) override;

    private:

        Common::Physics::PhysicsModule* physicsModule = nullptr;
        Common::Time::TimeModule* timeModule = nullptr;
        Common::Component::ComponentModule* componentModule = nullptr;
        Network::ServerNetworkModule* networkModule = nullptr;

        std::unordered_map<int32_t, ServerPlayerPhysicsState> playerStates;
        std::mutex inputMutex;

        struct SpawnPoint { glm::vec3 position{0.0f, 2.0f, 5.0f}; float yaw = 0.0f; };
        std::vector<SpawnPoint> spawnPoints;

        float broadcastAccumulator = 0.0f;
        static constexpr float BROADCAST_INTERVAL = 1.0f / 20.0f;  // 20Hz
    };
}
