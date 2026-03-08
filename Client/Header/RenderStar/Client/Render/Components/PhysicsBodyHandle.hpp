#pragma once

class btRigidBody;
class btKinematicCharacterController;
class btPairCachingGhostObject;

namespace RenderStar::Client::Render::Components
{
    struct PhysicsBodyHandle
    {
        btRigidBody* body = nullptr;
        btKinematicCharacterController* controller = nullptr;
        btPairCachingGhostObject* ghostObject = nullptr;
        float capsuleRadius = 0.4f;
        float capsuleHeight = 2.0f;
        bool isLocalPlayer = false;
    };
}
