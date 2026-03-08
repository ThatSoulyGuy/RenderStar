#include "RenderStar/Server/Physics/ServerPhysicsModule.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Network/Packets/PlayerInputPacket.hpp"
#include "RenderStar/Common/Network/Packets/PlayerStatePacket.hpp"
#include "RenderStar/Common/Physics/MovementModel.hpp"
#include "RenderStar/Common/Physics/PhysicsModule.hpp"
#include "RenderStar/Common/Time/TimeModule.hpp"
#include "RenderStar/Server/Network/ServerNetworkModule.hpp"

#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <algorithm>
#include <cmath>

namespace RenderStar::Server::Physics
{
    void ServerPhysicsModule::OnInitialize(Common::Module::ModuleContext& context)
    {
        physicsModule = &context.GetDependency<Common::Physics::PhysicsModule>();
        timeModule = &context.GetDependency<Common::Time::TimeModule>();
        componentModule = &context.GetDependency<Common::Component::ComponentModule>();
        networkModule = &context.GetDependency<Network::ServerNetworkModule>();

        logger->info("ServerPhysicsModule initialized");
    }

    void ServerPhysicsModule::OnPlayerJoined(int32_t playerId, Common::Component::GameObject entity,
                                              std::shared_ptr<Network::ClientConnection> connection,
                                              const glm::vec3& spawnPos)
    {
        auto result = physicsModule->CreateCharacterController(Common::Physics::PlayerDimensions::CAPSULE_RADIUS,
                                                               Common::Physics::PlayerDimensions::CAPSULE_HEIGHT, spawnPos);

        ServerPlayerPhysicsState state;
        state.playerId = playerId;
        state.entity = entity;
        state.controller = result.controller;
        state.ghostObject = result.ghostObject;
        state.connection = std::move(connection);

        playerStates[playerId] = std::move(state);

        // Log settled ghost position to verify settle fix is active
        if (result.ghostObject)
        {
            btTransform ghostTx = result.ghostObject->getWorldTransform();
            btVector3 origin = ghostTx.getOrigin();
            float totalHeight = Common::Physics::PlayerDimensions::TOTAL_HEIGHT;
            float feetY = origin.y() - totalHeight * 0.5f;
            bool grounded = result.controller ? result.controller->onGround() : false;
            logger->info("Player {} ghost after settle: feetY={:.2f}, ghostY={:.2f}, onGround={}, spawnPos=({:.2f},{:.2f},{:.2f})",
                playerId, feetY, origin.y(), grounded, spawnPos.x, spawnPos.y, spawnPos.z);
        }

        logger->info("Created physics state for player {}", playerId);
    }

    void ServerPhysicsModule::OnPlayerLeft(int32_t playerId)
    {
        auto it = playerStates.find(playerId);

        if (it == playerStates.end())
            return;

        auto& state = it->second;

        if (state.controller && state.ghostObject)
            physicsModule->RemoveCharacterController(state.controller, state.ghostObject);

        playerStates.erase(it);

        logger->info("Removed physics state for player {}", playerId);
    }

    void ServerPhysicsModule::QueueInput(int32_t playerId, int32_t sequenceNumber, uint8_t flags,
                                          float yaw, float pitch, float deltaTime)
    {
        std::lock_guard lock(inputMutex);

        auto it = playerStates.find(playerId);

        if (it == playerStates.end())
            return;

        QueuedInput input;
        input.sequenceNumber = sequenceNumber;
        input.flags = flags;
        input.yaw = yaw;
        input.pitch = pitch;
        input.deltaTime = std::clamp(deltaTime, 0.0f, 0.1f);

        it->second.inputQueue.push_back(input);
    }

    void ServerPhysicsModule::Tick()
    {
        // Drain queued inputs under lock
        std::unordered_map<int32_t, std::vector<QueuedInput>> drainedInputs;

        {
            std::lock_guard lock(inputMutex);

            for (auto& [playerId, state] : playerStates)
            {
                if (!state.inputQueue.empty())
                {
                    drainedInputs[playerId] = std::move(state.inputQueue);
                    state.inputQueue.clear();
                }
            }
        }

        // Process ALL queued inputs per player to keep velocity accumulation
        // in sync with the client (which runs ComputeMovement once per frame).
        for (auto& [playerId, inputs] : drainedInputs)
        {
            auto it = playerStates.find(playerId);

            if (it == playerStates.end())
                continue;

            auto& state = it->second;

            bool grounded = state.controller ? state.controller->onGround() : false;
            bool wantsJump = false;

            for (auto& input : inputs)
            {
                state.currentYaw = input.yaw;
                state.currentPitch = input.pitch;
                state.lastProcessedSequence = input.sequenceNumber;

                glm::vec2 curVel(state.currentVelocityX, state.currentVelocityZ);
                auto moveResult = Common::Physics::ComputeMovement(input.flags, input.yaw, input.deltaTime, curVel, grounded);

                state.currentVelocityX = moveResult.velocity.x;
                state.currentVelocityZ = moveResult.velocity.y;

                using Common::Network::Packets::PlayerInputPacket;
                if (input.flags & PlayerInputPacket::FLAG_JUMP)
                    wantsJump = true;
            }

            if (state.controller)
            {
                Common::Physics::MovementResult finalResult;
                finalResult.velocity = glm::vec2(state.currentVelocityX, state.currentVelocityZ);

                Common::Physics::ApplyMovement(state.controller, finalResult, wantsJump, grounded);
            }
        }

        // Step physics
        float dt = timeModule->GetDeltaTime();
        physicsModule->StepSimulation(dt);

        // Update server-side transforms every tick
        for (auto& [playerId, state] : playerStates)
        {
            if (!state.ghostObject)
                continue;

            btTransform ghostTransform = state.ghostObject->getWorldTransform();
            btVector3 origin = ghostTransform.getOrigin();

            float totalHeight = Common::Physics::PlayerDimensions::TOTAL_HEIGHT;

            glm::vec3 feetPos(origin.x(), origin.y() - totalHeight * 0.5f, origin.z());

            auto transformOpt = componentModule->GetComponent<Common::Component::Transform>(state.entity);

            if (transformOpt.has_value())
            {
                transformOpt->get().position = feetPos;
                componentModule->MarkEntityDirty(state.entity);
            }
        }

        // Broadcast state to clients at limited rate (20Hz)
        broadcastAccumulator += dt;

        if (broadcastAccumulator >= BROADCAST_INTERVAL)
        {
            broadcastAccumulator -= BROADCAST_INTERVAL;

            for (auto& [playerId, state] : playerStates)
            {
                if (!state.ghostObject)
                    continue;

                btTransform ghostTransform = state.ghostObject->getWorldTransform();
                btVector3 origin = ghostTransform.getOrigin();

                float totalHeight = Common::Physics::PlayerDimensions::TOTAL_HEIGHT;

                glm::vec3 feetPos(origin.x(), origin.y() - totalHeight * 0.5f, origin.z());

                Common::Network::Packets::PlayerStatePacket statePacket;
                statePacket.playerId = playerId;
                statePacket.lastProcessedSequence = state.lastProcessedSequence;
                statePacket.posX = feetPos.x;
                statePacket.posY = feetPos.y;
                statePacket.posZ = feetPos.z;
                statePacket.yaw = state.currentYaw;
                statePacket.pitch = state.currentPitch;
                statePacket.grounded = state.controller ? state.controller->onGround() : false;
                statePacket.serverTime = timeModule->GetElapsedTime();

                networkModule->Broadcast(statePacket);
            }
        }
    }

    void ServerPhysicsModule::AddSpawnPoint(const glm::vec3& position, float yaw)
    {
        spawnPoints.push_back({ position, yaw });
        logger->info("Added spawn point at ({:.1f}, {:.1f}, {:.1f}), yaw={:.0f}", position.x, position.y, position.z, yaw);
    }

    glm::vec3 ServerPhysicsModule::GetSpawnPosition() const
    {
        if (spawnPoints.empty())
            return glm::vec3(0.0f, 2.0f, 5.0f);

        return spawnPoints[0].position;
    }

    float ServerPhysicsModule::GetSpawnYaw() const
    {
        if (spawnPoints.empty())
            return 0.0f;

        return spawnPoints[0].yaw;
    }

    std::vector<std::type_index> ServerPhysicsModule::GetDependencies() const
    {
        return DependsOn<
            Common::Physics::PhysicsModule,
            Common::Time::TimeModule,
            Common::Component::ComponentModule,
            Network::ServerNetworkModule>();
    }
}
