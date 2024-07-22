#pragma once

#include "RenderStar/ECS/Component.hpp"
#include "RenderStar/Math/Vector3.hpp"
#include "RenderStar/Math/Quaternion.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::ECS;

namespace RenderStar
{
	namespace Math
	{
        class Transform : public Component
        {

        public:

            Transform(const Transform&) = delete;
            Transform& operator=(const Transform&) = delete;

            Vector3f GetPosition() const
            {
                return localPosition;
            }

            Quaternionf GetRotation() const
            {
                return localRotation;
            }

            Vector3f GetScale() const
            {
                return localScale;
            }

            Vector3f GetWorldPosition() const
            {
                auto worldMatrix = GetWorldMatrix(false);

                DirectX::XMFLOAT3 position;
                DirectX::XMStoreFloat3(&position, worldMatrix.r[3]);

                return Vector3f(position.x, position.y, position.z);
            }

            Quaternionf GetWorldRotation() const
            {
                auto worldMatrix = GetWorldMatrix(false);

                DirectX::XMVECTOR rotation = DirectX::XMQuaternionRotationMatrix(worldMatrix);
                DirectX::XMFLOAT4 rotationQuat;
                DirectX::XMStoreFloat4(&rotationQuat, rotation);

                return Quaternionf(rotationQuat.x, rotationQuat.y, rotationQuat.z, rotationQuat.w);
            }

            Vector3f GetWorldScale() const
            {
                auto worldMatrix = GetWorldMatrix(false);

                DirectX::XMVECTOR scale;
                DirectX::XMVECTOR rotation;
                DirectX::XMVECTOR translation;
                DirectX::XMMatrixDecompose(&scale, &rotation, &translation, worldMatrix);

                DirectX::XMFLOAT3 scaleVector;
                DirectX::XMStoreFloat3(&scaleVector, scale);

                return Vector3f(scaleVector.x, scaleVector.y, scaleVector.z);
            }

            Vector3f GetForward() const
			{
				auto worldMatrix = GetWorldMatrix(false);

				DirectX::XMFLOAT3 forward;
				DirectX::XMStoreFloat3(&forward, worldMatrix.r[2]);

				return Vector3f(forward.x, forward.y, forward.z);
			}

            Vector3f GetRight() const
            {
                auto worldMatrix = GetWorldMatrix(false);

                DirectX::XMFLOAT3 right;
                DirectX::XMStoreFloat3(&right, worldMatrix.r[0]);

                return Vector3f(right.x, right.y, right.z);
            }

            void SetPosition(const Vector3f& position)
            {
                localPosition = position;
            }

            void SetRotation(const Quaternionf& rotation)
            {
                localRotation = rotation;
            }

            void SetScale(const Vector3f& scale)
            {
                localScale = scale;
            }

            void SetParent(const Shared<Transform>& parent)
            {
                this->parent = parent;
            }

            Shared<Transform> GetParent() const
            {
                return parent;
            }

            DirectX::XMMATRIX GetWorldMatrix(bool transpose) const
            {
                DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(localPosition.x, localPosition.y, localPosition.z);
                DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(localRotation.x, localRotation.y, localRotation.z, localRotation.w));
                DirectX::XMMATRIX scaling = DirectX::XMMatrixScaling(localScale.x, localScale.y, localScale.z);

                DirectX::XMMATRIX localMatrix = scaling * rotation * translation;

                DirectX::XMMATRIX worldMatrix = localMatrix;

                if (parent != nullptr)
                { 
                    DirectX::XMMATRIX parentWorldMatrix = parent->GetWorldMatrix(transpose);
                    worldMatrix = localMatrix * parentWorldMatrix;
                }

                return transpose ? DirectX::XMMatrixTranspose(worldMatrix) : worldMatrix;
            }

            static Shared<Transform> Create()
            {
                class Enabled : public Transform { };

                return std::make_shared<Enabled>();
            }

        private:

            Transform() = default;

            Vector3f localPosition;
            Quaternionf localRotation;
            Vector3f localScale = Vector3f(1.0f, 1.0f, 1.0f);

            Shared<Transform> parent = nullptr;
        };
	}
}