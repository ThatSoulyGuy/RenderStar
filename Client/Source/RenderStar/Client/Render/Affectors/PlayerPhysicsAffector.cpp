#include "RenderStar/Client/Render/Affectors/PlayerPhysicsAffector.hpp"
#include "RenderStar/Client/Render/Components/PhysicsBodyHandle.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/PlayerIdentity.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Physics/MovementModel.hpp"
#include "RenderStar/Common/Physics/PhysicsModule.hpp"
#include "RenderStar/Common/Time/TimeModule.hpp"

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>

namespace RenderStar::Client::Render::Affectors
{
    using namespace Common::Component;

    void PlayerPhysicsAffector::OnInitialize(Common::Module::ModuleContext& context)
    {
        if (auto physics = context.GetModule<Common::Physics::PhysicsModule>(); physics.has_value())
            physicsModule = &physics->get();

        if (auto time = context.GetModule<Common::Time::TimeModule>(); time.has_value())
            timeModule = &time->get();

        if (!physicsModule)
            logger->error("PlayerPhysicsAffector: PhysicsModule not found");

        if (!timeModule)
            logger->error("PlayerPhysicsAffector: TimeModule not found");
    }

    void PlayerPhysicsAffector::Affect(ComponentModule& componentModule)
    {
        if (!physicsModule || !timeModule)
            return;

        float deltaTime = timeModule->GetDeltaTime();
        physicsModule->StepSimulation(deltaTime);

        // Read local player position from character controller ghost object
        auto& pool = componentModule.GetPool<Components::PhysicsBodyHandle>();

        for (auto [entity, handle] : pool)
        {
            if (!handle.isLocalPlayer || !handle.ghostObject)
                continue;

            auto transformOpt = componentModule.GetComponent<Transform>(entity);

            if (!transformOpt.has_value())
                continue;

            btTransform ghostTransform = handle.ghostObject->getWorldTransform();
            btVector3 origin = ghostTransform.getOrigin();

            float totalHeight = handle.capsuleHeight + 2.0f * handle.capsuleRadius;
            float feetY = origin.y() - totalHeight * 0.5f;

            // Diagnostic: log physics world state on first player frame
            if (!diagnosticLogged)
            {
                diagnosticLogged = true;

                int32_t objectCount = physicsModule->GetCollisionObjectCount();
                bool grounded = handle.controller ? handle.controller->onGround() : false;
                int32_t ghostOverlaps = handle.ghostObject->getNumOverlappingObjects();

                auto groundHit = physicsModule->RaycastGroundHeight(origin.x(), origin.z(), origin.y());

                logger->info("=== PHYSICS DIAGNOSTIC ===");
                logger->info("  Collision objects in world: {}", objectCount);
                logger->info("  Ghost position: ({:.2f}, {:.2f}, {:.2f})", origin.x(), origin.y(), origin.z());
                logger->info("  Feet Y: {:.2f}", feetY);
                logger->info("  onGround: {}", grounded);
                logger->info("  Ghost overlapping objects: {}", ghostOverlaps);
                logger->info("  Raycast ground hit: {}", groundHit.has_value() ? std::to_string(*groundHit) : "NONE");
                logger->info("  deltaTime: {:.4f}", deltaTime);
                logger->info("==========================");

                if (objectCount <= 1)
                    logger->error("NO STATIC COLLISION MESHES in physics world! Character will fall through floor.");

                if (!groundHit.has_value())
                    logger->error("Raycast found NO ground — collision mesh missing or at wrong position.");
            }

            // Per-frame diagnostic: detect falling through floor
            if (feetY < -2.0f && diagFrameCount < 5)
            {
                diagFrameCount++;
                int32_t objectCount = physicsModule->GetCollisionObjectCount();
                bool grounded = handle.controller ? handle.controller->onGround() : false;
                logger->error("Player falling! feetY={:.2f}, collisionObjects={}, onGround={}, dt={:.4f}",
                    feetY, objectCount, grounded, deltaTime);
            }

            // Safety: if character fell far, warp back
            if (feetY < -50.0f)
            {
                float safeY = 2.0f + totalHeight * 0.5f;
                handle.controller->warp(btVector3(origin.x(), safeY, origin.z()));
                physicsModule->ResetCharacterController(handle.controller, handle.ghostObject);
                origin = handle.ghostObject->getWorldTransform().getOrigin();
                feetY = origin.y() - totalHeight * 0.5f;
            }

            transformOpt->get().position = glm::vec3(origin.x(), feetY, origin.z());
            componentModule.MarkEntityDirty(entity);
        }
    }
}
