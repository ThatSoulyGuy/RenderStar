#pragma once

#include "RenderStar/Core/Window.hpp"
#include "RenderStar/ECS/GameObject.hpp"
#include "RenderStar/Math/Transform.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Core;
using namespace RenderStar::ECS;
using namespace RenderStar::Math;
using namespace RenderStar::Util;

namespace RenderStar
{
	namespace Render
	{
		class Camera : public Component
		{

		public:

			Camera(const Camera&) = delete;
			Camera& operator=(const Camera&) = delete;

			DirectX::XMMATRIX GetProjectionMatrix() const
			{
				return DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fieldOfView), Window::GetInstance()->GetAspectRatio(), nearPlane, farPlane);
			}

			DirectX::XMMATRIX GetViewMatrix() const
			{
				Shared<Transform> transform = gameObject->GetComponent<Transform>();
				Vector3f worldPosition = transform->GetWorldPosition();
				Vector3f forward = transform->GetForward();

				return DirectX::XMMatrixLookAtLH(worldPosition, worldPosition + forward, { 0.0f, 1.0f, 0.0f });
			}

			float GetNearPlane() const
			{
				return nearPlane;
			}

			float GetFarPlane() const
			{
				return farPlane;
			}

			float GetFieldOfView() const
			{
				return fieldOfView;
			}

			static Shared<Camera> Create(float nearPlane, float farPlane, float fieldOfView)
			{
				class Enabled : public Camera { };
				Shared<Camera> result = std::make_shared<Enabled>();

				result->nearPlane = nearPlane;
				result->farPlane = farPlane;
				result->fieldOfView = fieldOfView;

				return std::move(result);
			}

		private:

			Camera() = default;

			float nearPlane = 0.0f;
			float farPlane = 0.0f;
			float fieldOfView = 0.0f;
		};
	}
}