#include "RenderStar/Common/Physics/PhysicsModule.hpp"
#include "RenderStar/Common/Physics/MovementModel.hpp"

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <algorithm>

namespace RenderStar::Common::Physics
{
    PhysicsModule::PhysicsModule() = default;
    PhysicsModule::~PhysicsModule() = default;

    void PhysicsModule::OnInitialize(Module::ModuleContext&)
    {
        collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
        dispatcher = std::make_unique<btCollisionDispatcher>(collisionConfig.get());
        broadphase = std::make_unique<btDbvtBroadphase>();
        solver = std::make_unique<btSequentialImpulseConstraintSolver>();

        dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(
            dispatcher.get(), broadphase.get(), solver.get(), collisionConfig.get());

        dynamicsWorld->setGravity(btVector3(0, MovementConstants::GRAVITY, 0));

        ghostCallback = std::make_unique<btGhostPairCallback>();
        broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(ghostCallback.get());

        logger->info("PhysicsModule initialized");
    }

    void PhysicsModule::OnCleanup()
    {
        for (auto& controller : ownedControllers)
        {
            if (dynamicsWorld)
                dynamicsWorld->removeAction(controller.get());
        }
        ownedControllers.clear();

        for (auto& ghost : ownedGhosts)
        {
            if (dynamicsWorld)
                dynamicsWorld->removeCollisionObject(ghost.get());
        }
        ownedGhosts.clear();

        for (auto& body : ownedBodies)
        {
            if (dynamicsWorld)
                dynamicsWorld->removeRigidBody(body.get());
        }
        ownedBodies.clear();

        ownedShapes.clear();
        ownedMotionStates.clear();
        ownedMeshArrays.clear();
        ownedVertexData.clear();
        ownedIndexData.clear();
        dynamicsWorld.reset();
        solver.reset();
        broadphase.reset();
        dispatcher.reset();
        collisionConfig.reset();
        ghostCallback.reset();

        logger->info("PhysicsModule cleaned up");
    }

    void PhysicsModule::StepSimulation(float deltaTime)
    {
        if (!dynamicsWorld)
            return;

        // Clamp to prevent Bullet's m_localTime accumulation after frame spikes
        float clampedDt = std::min(deltaTime, 0.05f);
        dynamicsWorld->stepSimulation(clampedDt, 10, MovementConstants::PHYSICS_SUBSTEP);
    }

    btRigidBody* PhysicsModule::CreateKinematicCapsule(float radius, float height, const glm::vec3& position)
    {
        auto shape = std::make_unique<btCapsuleShape>(radius, height);

        btTransform startTransform;
        startTransform.setIdentity();
        float totalHeight = height + 2.0f * radius;
        startTransform.setOrigin(btVector3(position.x, position.y + totalHeight * 0.5f, position.z));

        auto motionState = std::make_unique<btDefaultMotionState>(startTransform);

        btRigidBody::btRigidBodyConstructionInfo info(0, motionState.get(), shape.get());
        info.m_startWorldTransform = startTransform;

        auto body = std::make_unique<btRigidBody>(info);
        body->setCollisionFlags(body->getCollisionFlags() |
            btCollisionObject::CF_KINEMATIC_OBJECT);
        body->setActivationState(DISABLE_DEACTIVATION);

        dynamicsWorld->addRigidBody(body.get());

        btRigidBody* rawPtr = body.get();
        ownedBodies.push_back(std::move(body));
        ownedShapes.push_back(std::move(shape));
        ownedMotionStates.push_back(std::move(motionState));

        return rawPtr;
    }

    CharacterControllerResult PhysicsModule::CreateCharacterController(float radius, float height, const glm::vec3& position)
    {
        auto shape = std::make_unique<btCapsuleShape>(radius, height);
        auto ghost = std::make_unique<btPairCachingGhostObject>();

        btTransform startTransform;
        startTransform.setIdentity();
        float totalHeight = height + 2.0f * radius;
        startTransform.setOrigin(btVector3(position.x, position.y + totalHeight * 0.5f, position.z));

        ghost->setWorldTransform(startTransform);
        ghost->setCollisionShape(shape.get());
        ghost->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

        float stepHeight = 0.35f;
        auto controller = std::make_unique<btKinematicCharacterController>(
            ghost.get(), shape.get(), stepHeight, btVector3(0, 1, 0));

        controller->setGravity(btVector3(0, MovementConstants::GRAVITY, 0));
        controller->setJumpSpeed(MovementConstants::JUMP_SPEED);
        controller->setFallSpeed(MovementConstants::FALL_SPEED);
        controller->setMaxSlope(btRadians(45.0f));
        controller->setUpInterpolate(true);
        controller->setMaxPenetrationDepth(0.04f);
        controller->setUseGhostSweepTest(false);

        dynamicsWorld->addCollisionObject(ghost.get(),
            btBroadphaseProxy::CharacterFilter,
            btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter | btBroadphaseProxy::CharacterFilter);
        dynamicsWorld->addAction(controller.get());

        // Use raycast to find actual ground and place character slightly above it.
        // This prevents btKinematicCharacterController::stepDown from sweeping
        // through the floor when the capsule starts exactly in-contact with it.
        auto groundHit = RaycastGroundHeight(position.x, position.z, position.y + 2.0f);

        if (groundHit.has_value())
        {
            float groundY = *groundHit;
            float correctedFeetY = groundY + 0.05f;

            btTransform correctedTransform;
            correctedTransform.setIdentity();
            correctedTransform.setOrigin(btVector3(position.x, correctedFeetY + totalHeight * 0.5f, position.z));
            ghost->setWorldTransform(correctedTransform);
            dynamicsWorld->updateSingleAabb(ghost.get());

            logger->info("CreateCharacterController: spawn=({:.2f},{:.2f},{:.2f}), groundHit={:.2f}, correctedFeetY={:.2f}",
                position.x, position.y, position.z, groundY, correctedFeetY);
        }
        else
        {
            logger->info("CreateCharacterController: spawn=({:.2f},{:.2f},{:.2f}), no ground raycast hit",
                position.x, position.y, position.z);
        }

        CharacterControllerResult result;
        result.controller = controller.get();
        result.ghostObject = ghost.get();

        ownedControllers.push_back(std::move(controller));
        ownedGhosts.push_back(std::move(ghost));
        ownedShapes.push_back(std::move(shape));

        return result;
    }

    void PhysicsModule::RemoveBody(btRigidBody* body)
    {
        if (!body || !dynamicsWorld)
            return;

        dynamicsWorld->removeRigidBody(body);

        auto it = std::find_if(ownedBodies.begin(), ownedBodies.end(),
            [body](const auto& ptr) { return ptr.get() == body; });

        if (it != ownedBodies.end())
            ownedBodies.erase(it);
    }

    void PhysicsModule::RemoveCharacterController(btKinematicCharacterController* controller, btPairCachingGhostObject* ghost)
    {
        if (!dynamicsWorld)
            return;

        if (controller)
        {
            dynamicsWorld->removeAction(controller);

            auto it = std::find_if(ownedControllers.begin(), ownedControllers.end(),
                [controller](const auto& ptr) { return ptr.get() == controller; });

            if (it != ownedControllers.end())
                ownedControllers.erase(it);
        }

        if (ghost)
        {
            dynamicsWorld->removeCollisionObject(ghost);

            auto it = std::find_if(ownedGhosts.begin(), ownedGhosts.end(),
                [ghost](const auto& ptr) { return ptr.get() == ghost; });

            if (it != ownedGhosts.end())
                ownedGhosts.erase(it);
        }
    }

    void PhysicsModule::SyncKinematicBody(btRigidBody* body, const glm::vec3& position)
    {
        if (!body)
            return;

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(position.x, position.y, position.z));
        body->getMotionState()->setWorldTransform(transform);
    }

    std::optional<float> PhysicsModule::RaycastGroundHeight(float x, float z, float startY) const
    {
        if (!dynamicsWorld)
            return std::nullopt;

        btVector3 from(x, startY + 2.0f, z);
        btVector3 to(x, startY - 20.0f, z);

        btCollisionWorld::ClosestRayResultCallback callback(from, to);
        callback.m_collisionFilterGroup = btBroadphaseProxy::AllFilter;
        callback.m_collisionFilterMask = btBroadphaseProxy::StaticFilter;

        dynamicsWorld->rayTest(from, to, callback);

        if (callback.hasHit())
            return callback.m_hitPointWorld.y();

        return std::nullopt;
    }

    void PhysicsModule::ResetCharacterController(btKinematicCharacterController* controller, btPairCachingGhostObject* ghost)
    {
        if (!dynamicsWorld)
            return;

        if (controller)
            controller->reset(dynamicsWorld.get());

        if (ghost)
            dynamicsWorld->updateSingleAabb(ghost);
    }

    void PhysicsModule::UpdateCharacterBroadphase(btPairCachingGhostObject* ghost)
    {
        if (!dynamicsWorld || !ghost)
            return;

        dynamicsWorld->updateSingleAabb(ghost);
    }

    int32_t PhysicsModule::GetCollisionObjectCount() const
    {
        if (!dynamicsWorld)
            return 0;
        return dynamicsWorld->getNumCollisionObjects();
    }

    void PhysicsModule::CreateStaticTriangleMesh(const float* vertexData, int32_t vertexCount,
                                                   int32_t vertexStride, const uint32_t* indices,
                                                   int32_t indexCount, float scale)
    {
        if (!dynamicsWorld || vertexCount == 0 || indexCount == 0)
            return;

        int32_t triangleCount = indexCount / 3;

        // Copy vertex positions with scale applied
        auto scaledVertices = std::make_unique<std::vector<float>>(vertexCount * 3);

        for (int32_t i = 0; i < vertexCount; ++i)
        {
            const float* src = vertexData + i * vertexStride;
            (*scaledVertices)[i * 3 + 0] = src[0] * scale;
            (*scaledVertices)[i * 3 + 1] = src[1] * scale;
            (*scaledVertices)[i * 3 + 2] = src[2] * scale;
        }

        // Copy indices as int32 (Bullet expects int, not uint32)
        auto intIndices = std::make_unique<std::vector<int32_t>>(indexCount);

        for (int32_t i = 0; i < indexCount; ++i)
            (*intIndices)[i] = static_cast<int32_t>(indices[i]);

        auto meshArray = std::make_unique<btTriangleIndexVertexArray>(
            triangleCount,
            intIndices->data(),
            3 * static_cast<int>(sizeof(int32_t)),
            vertexCount,
            scaledVertices->data(),
            3 * static_cast<int>(sizeof(float)));

        auto meshShape = std::make_unique<btBvhTriangleMeshShape>(meshArray.get(), true);

        btRigidBody::btRigidBodyConstructionInfo meshInfo(0, nullptr, meshShape.get());
        auto meshBody = std::make_unique<btRigidBody>(meshInfo);
        meshBody->setCollisionFlags(meshBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

        dynamicsWorld->addRigidBody(meshBody.get(),
            btBroadphaseProxy::StaticFilter,
            btBroadphaseProxy::CharacterFilter | btBroadphaseProxy::DefaultFilter);

        ownedBodies.push_back(std::move(meshBody));
        ownedShapes.push_back(std::move(meshShape));
        ownedMeshArrays.push_back(std::move(meshArray));
        ownedVertexData.push_back(std::move(scaledVertices));
        ownedIndexData.push_back(std::move(intIndices));
    }
}
