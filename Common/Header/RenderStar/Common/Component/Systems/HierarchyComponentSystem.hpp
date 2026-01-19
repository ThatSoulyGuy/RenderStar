#pragma once

#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Common/Component/Components/Hierarchy.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace RenderStar::Common::Component::Systems
{
    class HierarchyComponentSystem
    {
    public:

        void Run(ComponentModule& ecs)
        {
            if (!ecs.HasComponent<Transform>(GameObject{ 0 }) && ecs.GetPool<Transform>().Size() == 0)
                return;

            auto& transformPool = ecs.GetPool<Transform>();

            for (auto [entity, transform] : transformPool)
                BuildLocalMatrix(transform);

            if (ecs.GetPool<Hierarchy>().Size() > 0)
            {
                auto& hierarchyPool = ecs.GetPool<Hierarchy>();

                for (auto [entity, hierarchy] : hierarchyPool)
                {
                    if (!hierarchy.HasParent())
                        UpdateTransformRecursive(ecs, entity, glm::mat4(1.0f));
                }
            }

            for (auto [entity, transform] : transformPool)
            {
                if (!ecs.HasComponent<Hierarchy>(entity))
                {
                    transform.worldMatrix = transform.localMatrix;
                    DecomposeWorldMatrix(transform);
                }
            }
        }

    private:

        void BuildLocalMatrix(Transform& transform)
        {
            transform.localMatrix = glm::mat4(1.0f);
            transform.localMatrix = glm::translate(transform.localMatrix, transform.position);
            transform.localMatrix = transform.localMatrix * glm::mat4_cast(transform.rotation);
            transform.localMatrix = glm::scale(transform.localMatrix, transform.scale);
        }

        void UpdateTransformRecursive(ComponentModule& ecs, GameObject entity, const glm::mat4& parentWorldMatrix)
        {
            auto transformOpt = ecs.GetComponent<Transform>(entity);

            if (!transformOpt.has_value())
                return;

            Transform& transform = transformOpt.value().get();
            transform.worldMatrix = parentWorldMatrix * transform.localMatrix;
            DecomposeWorldMatrix(transform);

            auto hierarchyOpt = ecs.GetComponent<Hierarchy>(entity);

            if (!hierarchyOpt.has_value())
                return;

            Hierarchy& hierarchy = hierarchyOpt.value().get();

            for (GameObject child : hierarchy.children)
                UpdateTransformRecursive(ecs, child, transform.worldMatrix);
        }

        void DecomposeWorldMatrix(Transform& transform)
        {
            glm::mat4& m = transform.worldMatrix;

            transform.worldPosition = glm::vec3(m[3]);

            glm::vec3 col0(m[0][0], m[1][0], m[2][0]);
            glm::vec3 col1(m[0][1], m[1][1], m[2][1]);
            glm::vec3 col2(m[0][2], m[1][2], m[2][2]);

            transform.worldScale.x = glm::length(col0);
            transform.worldScale.y = glm::length(col1);
            transform.worldScale.z = glm::length(col2);

            if (transform.worldScale.x > 0.0f) col0 /= transform.worldScale.x;
            if (transform.worldScale.y > 0.0f) col1 /= transform.worldScale.y;
            if (transform.worldScale.z > 0.0f) col2 /= transform.worldScale.z;

            glm::mat3 rotationMatrix(
                col0.x, col1.x, col2.x,
                col0.y, col1.y, col2.y,
                col0.z, col1.z, col2.z
            );

            transform.worldRotation = glm::quat_cast(rotationMatrix);
        }
    };
}
