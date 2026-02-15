#include "RenderStar/Client/Gameplay/PlayerControllerAffector.hpp"
#include "RenderStar/Client/Gameplay/PlayerController.hpp"
#include "RenderStar/Client/Input/ClientInputModule.hpp"
#include "RenderStar/Common/Component/ComponentModule.hpp"
#include "RenderStar/Common/Component/Components/Transform.hpp"
#include "RenderStar/Common/Module/ModuleContext.hpp"
#include "RenderStar/Common/Time/TimeModule.hpp"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <algorithm>

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
        if (inputModule == nullptr || timeModule == nullptr)
            return;

        auto& pool = componentModule.GetPool<PlayerController>();

        if (pool.GetSize() == 0)
            return;

        const float deltaTime = timeModule->GetDeltaTime();

        for (auto [entity, controller] : pool)
        {
            auto transformOpt = componentModule.GetComponent<Transform>(entity);

            if (!transformOpt.has_value())
                continue;

            Transform& transform = transformOpt.value().get();

            if (inputModule->IsMouseButtonPressed(Input::MouseCode::RIGHT))
                inputModule->LockCursor();

            if (inputModule->IsMouseButtonReleased(Input::MouseCode::RIGHT))
                inputModule->UnlockCursor();

            if (inputModule->IsCursorLocked())
            {
                const glm::dvec2 delta = inputModule->GetCursorDelta();

                controller.yaw -= static_cast<float>(delta.x) * controller.lookSensitivity;
                controller.pitch -= static_cast<float>(delta.y) * controller.lookSensitivity;
                controller.pitch = std::clamp(controller.pitch, -89.0f, 89.0f);
            }

            const float yawRad = glm::radians(controller.yaw);
            const float pitchRad = glm::radians(controller.pitch);

            transform.rotation = glm::quat(glm::vec3(pitchRad, yawRad, 0.0f));

            const glm::vec3 forward(-glm::sin(yawRad), 0.0f, -glm::cos(yawRad));
            const glm::vec3 right(glm::cos(yawRad), 0.0f, -glm::sin(yawRad));
            constexpr glm::vec3 up(0.0f, 1.0f, 0.0f);

            glm::vec3 movement(0.0f);

            if (inputModule->IsKeyPressed(Input::KeyCode::W) || inputModule->IsKeyHeld(Input::KeyCode::W))
                movement += forward;

            if (inputModule->IsKeyPressed(Input::KeyCode::S) || inputModule->IsKeyHeld(Input::KeyCode::S))
                movement -= forward;

            if (inputModule->IsKeyPressed(Input::KeyCode::D) || inputModule->IsKeyHeld(Input::KeyCode::D))
                movement += right;

            if (inputModule->IsKeyPressed(Input::KeyCode::A) || inputModule->IsKeyHeld(Input::KeyCode::A))
                movement -= right;

            if (inputModule->IsKeyPressed(Input::KeyCode::SPACE) || inputModule->IsKeyHeld(Input::KeyCode::SPACE))
                movement += up;

            if (inputModule->IsKeyPressed(Input::KeyCode::LEFT_SHIFT) || inputModule->IsKeyHeld(Input::KeyCode::LEFT_SHIFT))
                movement -= up;

            if (glm::length(movement) > 0.0f)
                movement = glm::normalize(movement);

            transform.position += movement * controller.moveSpeed * deltaTime;
        }
    }
}
