#include "RenderStar/Client/Gameplay/ClientPlayerModule.hpp"
#include "RenderStar/Client/Gameplay/PlayerController.hpp"
#include "RenderStar/Client/Gameplay/PlayerControllerAffector.hpp"
#include "RenderStar/Client/Network/ClientNetworkModule.hpp"
#include "RenderStar/Client/Render/Components/Camera.hpp"
#include "RenderStar/Client/Render/Components/PhysicsBodyHandle.hpp"
#include "RenderStar/Client/Render/Components/RemotePlayerState.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/PlayerIdentity.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Network/PacketModule.hpp"
#include "RenderStar/Common/Network/Packets/PlayerAssignPacket.hpp"
#include "RenderStar/Common/Network/Packets/PlayerInputPacket.hpp"
#include "RenderStar/Common/Network/Packets/PlayerStatePacket.hpp"
#include "RenderStar/Common/Physics/MovementModel.hpp"
#include "RenderStar/Common/Physics/PhysicsModule.hpp"

#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

namespace RenderStar::Client::Gameplay
{
    void ClientPlayerModule::OnInitialize(Common::Module::ModuleContext& context)
    {
        auto& packetModule = context.GetDependency<Common::Network::PacketModule>();

        packetModule.RegisterHandler<Common::Network::Packets::PlayerAssignPacket>(
            [this](Common::Network::Packets::PlayerAssignPacket& packet)
            {
                localPlayerId.store(packet.playerId);
                logger->info("Assigned player ID: {}", packet.playerId);
            });

        packetModule.RegisterHandler<Common::Network::Packets::PlayerStatePacket>(
            [this](Common::Network::Packets::PlayerStatePacket& packet)
            {
                std::lock_guard lock(stateMutex);
                pendingStates.push(packet);
            });

        if (auto physics = context.GetModule<Common::Physics::PhysicsModule>(); physics.has_value())
            physicsModule = &physics->get();

        if (auto network = context.GetModule<Network::ClientNetworkModule>(); network.has_value())
            networkModule = &network->get();

        logger->info("ClientPlayerModule initialized");
    }

    int32_t ClientPlayerModule::GetLocalPlayerId() const
    {
        return localPlayerId.load();
    }

    Common::Component::GameObject ClientPlayerModule::GetLocalPlayerEntity() const
    {
        return localPlayerEntity;
    }

    void ClientPlayerModule::CheckForLocalPlayerEntity(Common::Component::ComponentModule& componentModule)
    {
        if (localPlayerSetUp)
            return;

        int32_t pid = localPlayerId.load();

        if (pid < 0)
            return;

        auto& pool = componentModule.GetPool<Common::Component::PlayerIdentity>();

        if (pool.GetSize() == 0)
        {
            logger->debug("Waiting for player entity: PlayerIdentity pool is empty (localPlayerId={})", pid);
            return;
        }

        for (auto [entity, identity] : pool)
        {
            if (identity.playerId != pid)
                continue;

            localPlayerEntity = entity;

            using Common::Physics::PlayerDimensions;

            auto& controller = componentModule.AddComponent<PlayerController>(entity);
            controller.lookSensitivity = 0.1f;

            auto& camera = componentModule.AddComponent<Render::Components::Camera>(entity);
            camera.projectionType = Render::Components::ProjectionType::PERSPECTIVE;
            camera.fieldOfView = 60.0f;
            camera.nearPlane = 0.1f;
            camera.farPlane = 100.0f;
            camera.eyeOffset = glm::vec3(0.0f, PlayerDimensions::EYE_HEIGHT, 0.0f);

            if (physicsModule)
            {
                auto transformOpt = componentModule.GetComponent<Common::Component::Transform>(entity);
                glm::vec3 spawnPos(0.0f, 2.0f, 0.0f);

                if (transformOpt.has_value())
                    spawnPos = transformOpt->get().position;

                auto result = physicsModule->CreateCharacterController(PlayerDimensions::CAPSULE_RADIUS, PlayerDimensions::CAPSULE_HEIGHT, spawnPos);

                auto& handle = componentModule.AddComponent<Render::Components::PhysicsBodyHandle>(entity);
                handle.controller = result.controller;
                handle.ghostObject = result.ghostObject;
                handle.capsuleRadius = PlayerDimensions::CAPSULE_RADIUS;
                handle.capsuleHeight = PlayerDimensions::CAPSULE_HEIGHT;
                handle.isLocalPlayer = true;
            }

            auto authority = componentModule.GetEntityAuthority(entity);
            logger->info("Local player entity set up: id={}, playerId={}, authority={}, ownerId={}", entity.id, pid, static_cast<int>(authority.level), authority.ownerId);

            localPlayerSetUp = true;
            break;
        }

        if (!localPlayerSetUp)
            logger->debug("PlayerIdentity pool has {} entries but none match localPlayerId={}", pool.GetSize(), pid);
    }

    void ClientPlayerModule::SendInputToServer(Common::Component::ComponentModule& componentModule)
    {
        if (!networkModule || !networkModule->IsConnected() || !playerControllerAffector)
            return;

        if (!localPlayerSetUp || !localPlayerEntity.IsValid())
            return;

        const auto& frameInput = playerControllerAffector->GetLastFrameInput();

        if (!frameInput.hasInput)
            return;

        Common::Network::Packets::PlayerInputPacket packet;
        packet.sequenceNumber = nextSequenceNumber++;
        packet.inputFlags = frameInput.flags;
        packet.yaw = frameInput.yaw;
        packet.pitch = frameInput.pitch;
        packet.deltaTime = frameInput.deltaTime;

        networkModule->Send(packet);

        // Store prediction
        auto transformOpt = componentModule.GetComponent<Common::Component::Transform>(localPlayerEntity);
        glm::vec3 predictedPos(0.0f);

        if (transformOpt.has_value())
            predictedPos = transformOpt->get().position;

        PredictedInput prediction;
        prediction.sequenceNumber = packet.sequenceNumber;
        prediction.flags = packet.inputFlags;
        prediction.yaw = packet.yaw;
        prediction.pitch = packet.pitch;
        prediction.deltaTime = packet.deltaTime;
        prediction.predictedPos = predictedPos;

        predictionBuffer.push_back(prediction);

        // Keep buffer bounded
        while (predictionBuffer.size() > 120)
            predictionBuffer.pop_front();
    }

    void ClientPlayerModule::ProcessServerStateUpdates(Common::Component::ComponentModule& componentModule)
    {
        std::queue<Common::Network::Packets::PlayerStatePacket> states;

        {
            std::lock_guard lock(stateMutex);
            states = std::move(pendingStates);
            pendingStates = {};
        }

        int32_t localPid = localPlayerId.load();

        while (!states.empty())
        {
            auto& state = states.front();

            if (state.playerId == localPid && localPlayerEntity.IsValid())
            {
                // Pop predictions up to server's last processed sequence,
                // saving the predicted position at that sequence for comparison
                glm::vec3 predictedPosAtServer(0.0f);
                bool hasPredictedPos = false;

                while (!predictionBuffer.empty() && predictionBuffer.front().sequenceNumber <= state.lastProcessedSequence)
                {
                    predictedPosAtServer = predictionBuffer.front().predictedPos;
                    hasPredictedPos = true;
                    predictionBuffer.pop_front();
                }

                glm::vec3 serverPos(state.posX, state.posY, state.posZ);

                auto transformOpt = componentModule.GetComponent<Common::Component::Transform>(localPlayerEntity);

                if (transformOpt.has_value())
                {
                    // Compare server position against the predicted position at the
                    // server's sequence number — NOT the current position (which
                    // includes unprocessed inputs the server hasn't seen yet)
                    glm::vec3 comparePos = hasPredictedPos
                        ? predictedPosAtServer
                        : transformOpt->get().position;

                    float error = glm::length(serverPos - comparePos);

                    if (error > 0.5f)
                    {
                        auto physicsOpt = componentModule.GetComponent<Render::Components::PhysicsBodyHandle>(localPlayerEntity);

                        if (physicsOpt.has_value() && physicsOpt->get().controller && physicsModule)
                        {
                            auto& handle = physicsOpt->get();
                            float totalHeight = handle.capsuleHeight + 2.0f * handle.capsuleRadius;

                            // Diagnostic: raycast to check if collision mesh supports
                            // the server's position on the client side
                            auto groundHit = physicsModule->RaycastGroundHeight(serverPos.x, serverPos.z, serverPos.y + 2.0f);
                            int32_t objCount = physicsModule->GetCollisionObjectCount();

                            logger->warn("Reconciliation diagnostic: collisionObjects={}, raycast from ({:.2f},{:.2f},{:.2f}) hit={}",
                                objCount, serverPos.x, serverPos.y + 2.0f, serverPos.z,
                                groundHit.has_value() ? std::to_string(*groundHit) : "NONE");

                            // Warp ghost to server position
                            handle.controller->warp(btVector3(serverPos.x, serverPos.y + totalHeight * 0.5f, serverPos.z));

                            // Reset vertical velocity + update broadphase. The reset()
                            // zeroes m_verticalVelocity (preventing accumulated fall speed
                            // from pushing through the floor) and the AABB update ensures
                            // broadphase pairs are correct at the new position.
                            physicsModule->ResetCharacterController(handle.controller, handle.ghostObject);

                            // Single recovery substep: reset() sets m_wasOnGround=false,
                            // so run one physics substep with zero walk direction to
                            // re-establish ground contact before any movement
                            handle.controller->setWalkDirection(btVector3(0, 0, 0));
                            physicsModule->StepSimulation(Common::Physics::MovementConstants::PHYSICS_SUBSTEP);

                            // Read position after recovery step
                            btTransform recoveryTx = handle.ghostObject->getWorldTransform();
                            btVector3 recoveryOrigin = recoveryTx.getOrigin();
                            float recoveryFeetY = recoveryOrigin.y() - totalHeight * 0.5f;
                            bool groundedAfterRecovery = handle.controller->onGround();

                            logger->warn("After warp+reset+recovery: feetY={:.2f}, onGround={}, ghostY={:.2f}",
                                recoveryFeetY, groundedAfterRecovery, recoveryOrigin.y());

                            // Replay unprocessed predictions to catch up to current state
                            if (!predictionBuffer.empty())
                            {
                                using Common::Network::Packets::PlayerInputPacket;
                                glm::vec2 replayVelocity(0.0f);

                                for (auto& pred : predictionBuffer)
                                {
                                    bool grounded = handle.controller->onGround();
                                    auto moveResult = Common::Physics::ComputeMovement(
                                        pred.flags, pred.yaw, pred.deltaTime, replayVelocity, grounded);
                                    replayVelocity = moveResult.velocity;

                                    bool wantsJump = pred.flags & PlayerInputPacket::FLAG_JUMP;
                                    Common::Physics::ApplyMovement(handle.controller, moveResult, wantsJump, grounded);
                                    physicsModule->StepSimulation(pred.deltaTime);
                                }

                                // Read replayed position
                                btTransform resultTx = handle.ghostObject->getWorldTransform();
                                btVector3 resultOrigin = resultTx.getOrigin();
                                float feetY = resultOrigin.y() - totalHeight * 0.5f;
                                transformOpt->get().position = glm::vec3(resultOrigin.x(), feetY, resultOrigin.z());
                            }
                            else
                            {
                                transformOpt->get().position = glm::vec3(recoveryOrigin.x(), recoveryFeetY, recoveryOrigin.z());
                            }
                        }
                        else
                        {
                            transformOpt->get().position = serverPos;
                        }

                        componentModule.MarkEntityDirty(localPlayerEntity);

                        logger->warn("Reconciliation: error={:.2f}, server=({:.2f},{:.2f},{:.2f}), predicted=({:.2f},{:.2f},{:.2f}), replayed {} inputs",
                            error, serverPos.x, serverPos.y, serverPos.z,
                            comparePos.x, comparePos.y, comparePos.z,
                            predictionBuffer.size());
                    }
                }
            }
            else
            {
                // Remote player: push snapshot for interpolation
                auto& identityPool = componentModule.GetPool<Common::Component::PlayerIdentity>();

                for (auto [entity, identity] : identityPool)
                {
                    if (identity.playerId != state.playerId)
                        continue;

                    if (!componentModule.HasComponent<Render::Components::RemotePlayerState>(entity))
                    {
                        componentModule.AddComponent<Render::Components::RemotePlayerState>(entity);

                        // Create a kinematic capsule so the local player collides with this remote player
                        if (physicsModule && !componentModule.HasComponent<Render::Components::PhysicsBodyHandle>(entity))
                        {
                            using Common::Physics::PlayerDimensions;
                            glm::vec3 remotePos(state.posX, state.posY, state.posZ);
                            auto* body = physicsModule->CreateKinematicCapsule(
                                PlayerDimensions::CAPSULE_RADIUS, PlayerDimensions::CAPSULE_HEIGHT, remotePos);

                            auto& handle = componentModule.AddComponent<Render::Components::PhysicsBodyHandle>(entity);
                            handle.body = body;
                            handle.capsuleRadius = PlayerDimensions::CAPSULE_RADIUS;
                            handle.capsuleHeight = PlayerDimensions::CAPSULE_HEIGHT;
                            handle.isLocalPlayer = false;
                        }
                    }

                    auto remoteOpt = componentModule.GetComponent<Render::Components::RemotePlayerState>(entity);

                    if (remoteOpt.has_value())
                    {
                        auto& remoteState = remoteOpt->get();

                        Render::Components::RemotePlayerState::Snapshot snap;
                        snap.position = glm::vec3(state.posX, state.posY, state.posZ);
                        snap.yaw = state.yaw;
                        snap.pitch = state.pitch;
                        snap.serverTime = state.serverTime;

                        remoteState.snapshots.push_back(snap);

                        while (remoteState.snapshots.size() > Render::Components::RemotePlayerState::MAX_SNAPSHOTS)
                            remoteState.snapshots.pop_front();
                    }

                    break;
                }
            }

            states.pop();
        }
    }

    std::vector<std::type_index> ClientPlayerModule::GetDependencies() const
    {
        return DependsOn<Common::Network::PacketModule>();
    }
}
