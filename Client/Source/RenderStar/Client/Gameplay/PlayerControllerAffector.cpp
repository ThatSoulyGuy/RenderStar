#include "RenderStar/Client/Gameplay/PlayerControllerAffector.hpp"
#include "RenderStar/Client/Gameplay/PlayerController.hpp"
#include "RenderStar/Client/Input/ClientInputModule.hpp"
#include "RenderStar/Client/Render/Components/Camera.hpp"
#include "RenderStar/Client/Render/Components/PhysicsBodyHandle.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Network/Packets/PlayerInputPacket.hpp"
#include "RenderStar/Common/Physics/MovementModel.hpp"
#include "RenderStar/Common/Time/TimeModule.hpp"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <algorithm>
#include <cmath>

#include <BulletDynamics/Character/btKinematicCharacterController.h>

namespace RenderStar::Client::Gameplay
{
    using namespace Common::Component;

    void PlayerControllerAffector::OnInitialize(Common::Module::ModuleContext& moduleContext)
    {
        if (const auto input = moduleContext.GetModule<Input::ClientInputModule>(); input.has_value())
            inputModule = &input->get();

        if (const auto time = moduleContext.GetModule<Common::Time::TimeModule>(); time.has_value())
            timeModule = &time->get();

        if (inputModule == nullptr || timeModule == nullptr)
            logger->error("PlayerControllerAffector: required modules not found");
    }

    void PlayerControllerAffector::Affect(ComponentModule& componentModule)
    {
        using Common::Network::Packets::PlayerInputPacket;

        lastFrameInput = FrameInput{};

        if (inputModule == nullptr || timeModule == nullptr)
            return;

        auto& pool = componentModule.GetPool<PlayerController>();

        if (pool.GetSize() == 0)
            return;

        // Clamp deltaTime to match the server-side clamp in QueueInput,
        // so ComputeMovement produces identical velocities on both sides
        const float deltaTime = std::min(timeModule->GetDeltaTime(), 0.1f);

        for (auto [entity, controller] : pool)
        {
            if (!componentModule.CheckAuthority(entity, GetAuthorityContext()))
            {
                if (!authorityWarningLogged)
                {
                    auto entityAuth = componentModule.GetEntityAuthority(entity);
                    logger->warn("PlayerControllerAffector: authority check failed for entity {} (entity authority={}, ownerId={}, caller authority={}, callerId={})",
                        entity.id, static_cast<int>(entityAuth.level), entityAuth.ownerId,
                        static_cast<int>(GetAuthorityContext().level), GetAuthorityContext().ownerId);
                    authorityWarningLogged = true;
                }
                continue;
            }

            auto transformOpt = componentModule.GetComponent<Transform>(entity);

            if (!transformOpt.has_value())
                continue;

            Transform& transform = transformOpt.value().get();
            bool changed = false;

            if (inputModule->IsMouseButtonPressed(Input::MouseCode::RIGHT))
                inputModule->LockCursor();

            if (inputModule->IsMouseButtonReleased(Input::MouseCode::RIGHT))
                inputModule->UnlockCursor();

            if (inputModule->IsCursorLocked())
            {
                const glm::dvec2 delta = inputModule->GetCursorDelta();

                if (delta.x != 0.0 || delta.y != 0.0)
                {
                    controller.yaw -= static_cast<float>(delta.x) * controller.lookSensitivity;
                    controller.pitch -= static_cast<float>(delta.y) * controller.lookSensitivity;
                    controller.pitch = std::clamp(controller.pitch, -89.0f, 89.0f);
                    changed = true;
                }
            }

            const float yawRad = glm::radians(controller.yaw);
            const float pitchRad = glm::radians(controller.pitch);

            transform.rotation = glm::quat(glm::vec3(pitchRad, yawRad, 0.0f));

            const glm::vec3 forward(-glm::sin(yawRad), 0.0f, -glm::cos(yawRad));
            const glm::vec3 right(glm::cos(yawRad), 0.0f, -glm::sin(yawRad));

            glm::vec3 movement(0.0f);
            uint8_t inputFlags = 0;

            if (inputModule->IsKeyPressed(Input::KeyCode::W) || inputModule->IsKeyHeld(Input::KeyCode::W))
            {
                movement += forward;
                inputFlags |= PlayerInputPacket::FLAG_FORWARD;
            }

            if (inputModule->IsKeyPressed(Input::KeyCode::S) || inputModule->IsKeyHeld(Input::KeyCode::S))
            {
                movement -= forward;
                inputFlags |= PlayerInputPacket::FLAG_BACKWARD;
            }

            if (inputModule->IsKeyPressed(Input::KeyCode::D) || inputModule->IsKeyHeld(Input::KeyCode::D))
            {
                movement += right;
                inputFlags |= PlayerInputPacket::FLAG_RIGHT;
            }

            if (inputModule->IsKeyPressed(Input::KeyCode::A) || inputModule->IsKeyHeld(Input::KeyCode::A))
            {
                movement -= right;
                inputFlags |= PlayerInputPacket::FLAG_LEFT;
            }

            if (glm::length(movement) > 0.0f)
                movement = glm::normalize(movement);

            auto physicsOpt = componentModule.GetComponent<Render::Components::PhysicsBodyHandle>(entity);

            if (physicsOpt.has_value() && physicsOpt->get().controller)
            {
                auto& handle = physicsOpt->get();
                bool grounded = handle.controller->onGround();

                // Shared movement model (matches server exactly)
                auto moveResult = Common::Physics::ComputeMovement(inputFlags, controller.yaw, deltaTime, controller.currentVelocity, grounded);
                controller.currentVelocity = moveResult.velocity;

                bool wantsJump = inputModule->IsKeyPressed(Input::KeyCode::SPACE) || inputModule->IsKeyHeld(Input::KeyCode::SPACE);
                Common::Physics::ApplyMovement(handle.controller, moveResult, wantsJump, grounded);

                if (wantsJump)
                    inputFlags |= PlayerInputPacket::FLAG_JUMP;

                // Head bob — only when grounded and moving
                float speed = glm::length(controller.currentVelocity);

                if (grounded && speed > 0.5f)
                {
                    float bobFrequency = 10.0f;
                    float bobAmplitude = 0.035f * std::min(speed / Common::Physics::MovementConstants::MOVE_SPEED, 1.0f);
                    controller.bobTimer += deltaTime * bobFrequency;
                    controller.bobAmount = std::sin(controller.bobTimer) * bobAmplitude;
                }
                else
                {
                    // Smoothly return to zero
                    controller.bobAmount *= std::max(0.0f, 1.0f - deltaTime * 8.0f);

                    if (std::abs(controller.bobAmount) < 0.001f)
                    {
                        controller.bobAmount = 0.0f;
                        controller.bobTimer = 0.0f;
                    }
                }

                // Write head bob to camera
                auto cameraOpt = componentModule.GetComponent<Render::Components::Camera>(entity);

                if (cameraOpt.has_value())
                    cameraOpt->get().eyeOffset.y = Common::Physics::PlayerDimensions::EYE_HEIGHT + controller.bobAmount;

                changed = glm::length(controller.currentVelocity) > 0.01f || changed;

                // Capture frame input for network sending
                lastFrameInput.flags = inputFlags;
                lastFrameInput.yaw = controller.yaw;
                lastFrameInput.pitch = controller.pitch;
                lastFrameInput.deltaTime = deltaTime;
                lastFrameInput.hasInput = true;
            }
            else
            {
                constexpr glm::vec3 up(0.0f, 1.0f, 0.0f);

                if (inputModule->IsKeyPressed(Input::KeyCode::SPACE) || inputModule->IsKeyHeld(Input::KeyCode::SPACE))
                    movement += up;

                if (inputModule->IsKeyPressed(Input::KeyCode::LEFT_SHIFT) || inputModule->IsKeyHeld(Input::KeyCode::LEFT_SHIFT))
                    movement -= up;

                if (glm::length(movement) > 0.0f)
                {
                    movement = glm::normalize(movement);
                    changed = true;
                }

                transform.position += movement * Common::Physics::MovementConstants::MOVE_SPEED * deltaTime;
            }

            if (changed)
                componentModule.MarkEntityDirty(entity);
        }
    }
}
