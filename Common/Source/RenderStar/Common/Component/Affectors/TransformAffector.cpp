#include "RenderStar/Common/Component/Affectors/TransformAffector.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Common/Component/Components/Hierarchy.hpp"
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace RenderStar::Common::Component::Affectors
{
    void TransformAffector::Affect(ComponentModule& componentModule)
    {
        auto& transformPool = componentModule.GetPool<Transform>();

        for (auto [entity, transform] : transformPool)
        {
            transform.localMatrix = glm::mat4(1.0f);
            transform.localMatrix = glm::translate(transform.localMatrix, transform.position);
            transform.localMatrix = transform.localMatrix * glm::toMat4(transform.rotation);
            transform.localMatrix = glm::scale(transform.localMatrix, transform.scale);

            transform.worldMatrix = transform.localMatrix;
            transform.worldPosition = transform.position;
            transform.worldRotation = transform.rotation;
            transform.worldScale = transform.scale;
        }

        auto& hierarchyPool = componentModule.GetPool<Hierarchy>();

        for (auto [entity, hierarchy] : hierarchyPool)
        {
            if (!hierarchy.HasParent())
                continue;

            if (!componentModule.HasComponent<Transform>(entity))
                continue;

            if (!componentModule.HasComponent<Transform>(hierarchy.parent))
                continue;

            auto transformOpt = componentModule.GetComponent<Transform>(entity);
            auto parentTransformOpt = componentModule.GetComponent<Transform>(hierarchy.parent);

            if (!transformOpt.has_value() || !parentTransformOpt.has_value())
                continue;

            Transform& transform = transformOpt.value().get();
            Transform& parentTransform = parentTransformOpt.value().get();

            transform.localMatrix = glm::mat4(1.0f);
            transform.localMatrix = glm::translate(transform.localMatrix, transform.position);
            transform.localMatrix = transform.localMatrix * glm::toMat4(transform.rotation);
            transform.localMatrix = glm::scale(transform.localMatrix, transform.scale);

            transform.worldMatrix = parentTransform.worldMatrix * transform.localMatrix;
            transform.worldRotation = parentTransform.worldRotation * transform.rotation;

            glm::vec3 rotatedPosition = parentTransform.worldRotation * transform.position;
            transform.worldPosition = rotatedPosition + parentTransform.worldPosition;

            transform.worldScale = parentTransform.worldScale * transform.scale;
        }
    }
}
