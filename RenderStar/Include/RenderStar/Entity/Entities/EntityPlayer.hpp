#pragma once

#include "RenderStar/Core/InputManager.hpp"
#include "RenderStar/ECS/GameObjectManager.hpp"
#include "RenderStar/Entity/Entity.hpp"
#include "RenderStar/Render/Camera.hpp"
#include "RenderStar/Render/Renderer.hpp"

using namespace RenderStar::Core;
using namespace RenderStar::Render;
using namespace RenderStar::Entity;

namespace RenderStar
{
	namespace Entity
	{
		class EntityPlayer : public IEntity
		{

		public:

			virtual void Initialize() override
			{
				InputManager::GetInstance()->SetCursorState(CursorState::LOCKED);

				gameObject->AddChild(GameObjectManager::GetInstance()->Register(GameObject::Create("Camera")));
				gameObject->GetChild("Camera")->AddComponent(Camera::Create(0.01f, 1000.0f, 45.0f));	

				Renderer::GetInstance()->SetCamera(gameObject->GetChild("Camera")->GetComponent<Camera>());
			}

			virtual void Update() override
			{
				UpdateMouseLook();
				UpdateMovement();
			}

		protected:

			friend class IEntity;

			virtual EntityRegistration GetRegistration() const override
			{
				return EntityRegistration::New()
					.SetRegistryName("entity_player")
					.SetMaxHealth(100.0f)
					.SetMovementSpeed(1.0f)
					.SetRunningSpeed(5.0f)
					.SetCanJump(true)
					.SetCanCrouch(true)
					.Build();
			}

		private:

			void UpdateMouseLook()
			{
				Vector2i mouseDelta = InputManager::GetInstance()->GetMouseDelta();
				const float sensitivity = 0.1f;

				float yaw = static_cast<float>(mouseDelta.x) * sensitivity;
				float pitch = static_cast<float>(mouseDelta.y) * sensitivity;

				auto transform = gameObject->GetChild("Camera")->GetComponent<Camera>()->gameObject->GetComponent<Transform>();

				Vector3f eulerRotation = transform->GetRotation();

				eulerRotation.y += yaw;
				eulerRotation.x += pitch;

				if (eulerRotation.x > 89.0f)
					eulerRotation.x = 89.0f;

				if (eulerRotation.x < -89.0f)
					eulerRotation.x = -89.0f;

				transform->SetRotation(eulerRotation);
			}

			void UpdateMovement()
			{
				Shared<Transform> cameraTransform = gameObject->GetChild("Camera")->GetComponent<Transform>();
				Vector3f movement = { 0.0f, 0.0f, 0.0f };

				if (InputManager::GetInstance()->GetKeyState(KeyCode::W, KeyState::HELD))
					movement = (movement.z + GetMovementSpeed()) * cameraTransform->GetForward();

				if (InputManager::GetInstance()->GetKeyState(KeyCode::S, KeyState::HELD))
					movement = (movement.z - GetMovementSpeed()) * cameraTransform->GetForward();

				if (InputManager::GetInstance()->GetKeyState(KeyCode::A, KeyState::HELD))
					movement = (movement.x - GetMovementSpeed()) * cameraTransform->GetRight();

				if (InputManager::GetInstance()->GetKeyState(KeyCode::D, KeyState::HELD))
					movement = (movement.x + GetMovementSpeed()) * cameraTransform->GetRight();

				gameObject->GetComponent<Transform>()->Translate(movement);
			}
		};
	}
}	