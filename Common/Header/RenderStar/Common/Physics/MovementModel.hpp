#pragma once

#include "RenderStar/Common/Network/Packets/PlayerInputPacket.hpp"
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <LinearMath/btVector3.h>
#include <cmath>
#include <glm/glm.hpp>

namespace RenderStar::Common::Physics
{
    struct PlayerDimensions
    {
        static constexpr float CAPSULE_RADIUS = 0.4f;
        static constexpr float CAPSULE_HEIGHT = 1.95f;
        static constexpr float TOTAL_HEIGHT = CAPSULE_HEIGHT + 2.0f * CAPSULE_RADIUS;  // 2.8
        static constexpr float EYE_HEIGHT = TOTAL_HEIGHT * 0.9f;                        // 2.52
        static constexpr float STEP_HEIGHT = 0.35f;
    };

    struct MovementConstants
    {
        static constexpr float MOVE_SPEED = 4.5f;
        static constexpr float GROUND_ACCEL = 35.0f;
        static constexpr float GROUND_FRICTION = 14.0f;
        static constexpr float AIR_ACCEL = 2.0f;
        static constexpr float JUMP_SPEED = 7.0f;
        static constexpr float GRAVITY = -20.0f;
        static constexpr float FALL_SPEED = 40.0f;
        static constexpr float VELOCITY_CUTOFF = 0.01f;
        static constexpr float PHYSICS_SUBSTEP = 1.0f / 120.0f;
    };

    struct MovementResult
    {
        glm::vec2 velocity{0.0f};
    };

    inline glm::vec3 ComputeInputDirection(uint8_t flags, float yaw)
    {
        float yawRad = glm::radians(yaw);
        glm::vec3 forward(-std::sin(yawRad), 0.0f, -std::cos(yawRad));
        glm::vec3 right(std::cos(yawRad), 0.0f, -std::sin(yawRad));

        glm::vec3 movement(0.0f);

        using Network::Packets::PlayerInputPacket;

        if (flags & PlayerInputPacket::FLAG_FORWARD)  movement += forward;
        if (flags & PlayerInputPacket::FLAG_BACKWARD) movement -= forward;
        if (flags & PlayerInputPacket::FLAG_RIGHT)    movement += right;
        if (flags & PlayerInputPacket::FLAG_LEFT)     movement -= right;

        if (glm::length(movement) > 0.0f)
            movement = glm::normalize(movement);

        return movement;
    }

    inline MovementResult ComputeMovement(uint8_t flags, float yaw, float deltaTime,
                                           glm::vec2 currentVelocity, bool grounded)
    {
        glm::vec3 direction = ComputeInputDirection(flags, yaw);

        glm::vec2 desiredVel(direction.x * MovementConstants::MOVE_SPEED,
                             direction.z * MovementConstants::MOVE_SPEED);

        if (glm::length(desiredVel) > 0.001f)
        {
            float accel = grounded ? MovementConstants::GROUND_ACCEL : MovementConstants::AIR_ACCEL;
            float factor = 1.0f - std::exp(-accel * deltaTime);
            currentVelocity = glm::mix(currentVelocity, desiredVel, factor);
        }
        else
        {
            float factor = 1.0f - std::exp(-MovementConstants::GROUND_FRICTION * deltaTime);
            currentVelocity = glm::mix(currentVelocity, glm::vec2(0.0f), factor);
        }

        if (glm::length(currentVelocity) < MovementConstants::VELOCITY_CUTOFF)
            currentVelocity = glm::vec2(0.0f);

        MovementResult result;
        result.velocity = currentVelocity;
        return result;
    }

    inline void ApplyMovement(btKinematicCharacterController* controller,
                               const MovementResult& movement,
                               bool wantsJump, bool grounded)
    {
        if (!controller)
            return;

        controller->setWalkDirection(
            btVector3(movement.velocity.x * MovementConstants::PHYSICS_SUBSTEP,
                      0,
                      movement.velocity.y * MovementConstants::PHYSICS_SUBSTEP));

        if (wantsJump && grounded)
            controller->jump(btVector3(0, MovementConstants::JUMP_SPEED, 0));
    }
}
