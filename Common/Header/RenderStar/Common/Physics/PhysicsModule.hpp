#pragma once

#include "RenderStar/Common/Module/AbstractModule.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <vector>

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btRigidBody;
class btCollisionShape;
class btTriangleIndexVertexArray;
class btKinematicCharacterController;
class btPairCachingGhostObject;
class btGhostPairCallback;
class btMotionState;

namespace RenderStar::Common::Physics
{
    struct CharacterControllerResult
    {
        btKinematicCharacterController* controller = nullptr;
        btPairCachingGhostObject* ghostObject = nullptr;
    };

    class PhysicsModule final : public Module::AbstractModule
    {
    public:

        PhysicsModule();
        ~PhysicsModule() override;

        void StepSimulation(float deltaTime);

        btRigidBody* CreateKinematicCapsule(float radius, float height, const glm::vec3& position);
        CharacterControllerResult CreateCharacterController(float radius, float height, const glm::vec3& position);

        void RemoveBody(btRigidBody* body);
        void RemoveCharacterController(btKinematicCharacterController* controller, btPairCachingGhostObject* ghost);

        void SyncKinematicBody(btRigidBody* body, const glm::vec3& position);

        void CreateStaticTriangleMesh(const float* vertexData, int32_t vertexCount,
                                       int32_t vertexStride, const uint32_t* indices,
                                       int32_t indexCount, float scale);

        std::optional<float> RaycastGroundHeight(float x, float z, float startY) const;

        void ResetCharacterController(btKinematicCharacterController* controller, btPairCachingGhostObject* ghost);

        void UpdateCharacterBroadphase(btPairCachingGhostObject* ghost);

        int32_t GetCollisionObjectCount() const;

    protected:

        void OnInitialize(Module::ModuleContext& context) override;
        void OnCleanup() override;

    private:

        std::unique_ptr<btDefaultCollisionConfiguration> collisionConfig;
        std::unique_ptr<btCollisionDispatcher> dispatcher;
        std::unique_ptr<btBroadphaseInterface> broadphase;
        std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
        std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;
        std::unique_ptr<btGhostPairCallback> ghostCallback;

        std::vector<std::unique_ptr<btCollisionShape>> ownedShapes;
        std::vector<std::unique_ptr<btMotionState>> ownedMotionStates;
        std::vector<std::unique_ptr<btRigidBody>> ownedBodies;
        std::vector<std::unique_ptr<btKinematicCharacterController>> ownedControllers;
        std::vector<std::unique_ptr<btPairCachingGhostObject>> ownedGhosts;
        std::vector<std::unique_ptr<btTriangleIndexVertexArray>> ownedMeshArrays;
        std::vector<std::unique_ptr<std::vector<float>>> ownedVertexData;
        std::vector<std::unique_ptr<std::vector<int32_t>>> ownedIndexData;
    };
}
