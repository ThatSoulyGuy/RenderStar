#pragma once

#include "RenderStar/Core/Logger.hpp"
#include "RenderStar/ECS/GameObject.hpp"
#include "RenderStar/Math/Transform.hpp"
#include "RenderStar/Render/Renderer.hpp"
#include "RenderStar/Render/Shader.hpp"
#include "RenderStar/Render/Texture.hpp"
#include "RenderStar/Render/Vertex.hpp"
#include "RenderStar/Util/Core/String.hpp"
#include "RenderStar/Util/Core/Vector.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Core;
using namespace RenderStar::ECS;
using namespace RenderStar::Util;

namespace RenderStar
{
	namespace Render
	{
		struct DefaultMatrixBuffer
		{
			DirectX::XMMATRIX projectionMatrix;
			DirectX::XMMATRIX viewMatrix;
			DirectX::XMMATRIX worldMatrix;
		};

		class Mesh : public Component
		{

		public:

			Mesh(const Mesh&) = delete;
			Mesh& operator=(const Mesh&) = delete;

			void Generate()
			{
				ComPtr<ID3D11Device> device = Renderer::GetInstance()->GetDevice();

				D3D11_BUFFER_DESC vertexBufferDescription = { };

				vertexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
				vertexBufferDescription.ByteWidth = sizeof(Vertex) * vertices.Length();
				vertexBufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				vertexBufferDescription.CPUAccessFlags = 0;

				D3D11_SUBRESOURCE_DATA vertexBufferData = { };

				vertexBufferData.pSysMem = vertices;
				vertexBufferData.SysMemPitch = 0;
				vertexBufferData.SysMemSlicePitch = 0;

				HRESULT result = device->CreateBuffer(&vertexBufferDescription, &vertexBufferData, &vertexBuffer);

				if (FAILED(result))
				{
					Logger_ThrowException("Failed to create vertex buffer.", false);
					return;
				}

				D3D11_BUFFER_DESC indexBufferDescription = { };

				indexBufferDescription.Usage = D3D11_USAGE_DEFAULT;
				indexBufferDescription.ByteWidth = sizeof(uint32) * indices.Length();
				indexBufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
				indexBufferDescription.CPUAccessFlags = 0;

				D3D11_SUBRESOURCE_DATA indexBufferData = { };

				indexBufferData.pSysMem = indices;
				indexBufferData.SysMemPitch = 0;
				indexBufferData.SysMemSlicePitch = 0;

				result = device->CreateBuffer(&indexBufferDescription, &indexBufferData, &indexBuffer);

				if (FAILED(result))
				{
					Logger_ThrowException("Failed to create index buffer.", false);
					return;
				}
			}

			void DebugMatrix(const DirectX::XMMATRIX& matrix, const String& name)
			{
				DirectX::XMFLOAT4X4 m;
				DirectX::XMStoreFloat4x4(&m, matrix);

				std::cout << "Matrix: " << name << std::endl;
				std::cout << m._11 << ", " << m._12 << ", " << m._13 << ", " << m._14 << std::endl;
				std::cout << m._21 << ", " << m._22 << ", " << m._23 << ", " << m._24 << std::endl;
				std::cout << m._31 << ", " << m._32 << ", " << m._33 << ", " << m._34 << std::endl;
				std::cout << m._41 << ", " << m._42 << ", " << m._43 << ", " << m._44 << std::endl;
			}

			void Render(Shared<Camera> camera) override
			{
				if (camera == nullptr)
					return;

				Shared<Shader> shader = gameObject->GetComponent<Shader>();
				Shared<Texture> texture = gameObject->GetComponent<Texture>();

				ComPtr<ID3D11DeviceContext> context = Renderer::GetInstance()->GetContext();
				ComPtr<ID3D11Device> device = Renderer::GetInstance()->GetDevice();

				UINT stride = sizeof(Vertex);
				UINT offset = 0;

				shader->Bind();

				shader->SetConstantBuffer(0, DefaultMatrixBuffer
				{
					DirectX::XMMatrixTranspose(camera->GetProjectionMatrix()),
					DirectX::XMMatrixTranspose(camera->GetViewMatrix()),
					gameObject->GetComponent<Transform>()->GetWorldMatrix(true),
				}, ShaderType::VERTEX);

				shader->SetShaderResourceView(0, texture->GetTextureView(), ShaderType::PIXEL);
				
				shader->SetSamplerState(0, texture->GetFilter(), texture->GetAddressMode(), ShaderType::PIXEL);

				context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
				context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
				context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				context->DrawIndexed(indices.Length(), 0, 0);
			}

			void CleanUp() override
			{
				vertexBuffer.Reset();
				indexBuffer.Reset();
			}

			static Shared<Mesh> Create(const String& name, const Vector<Vertex>& vertices, const Vector<uint32>& indices)
			{
				class Enabled : public Mesh { };
				Shared<Mesh> mesh = std::make_shared<Enabled>();

				mesh->name = name;
				mesh->vertices = vertices;
				mesh->indices = indices;

				return std::move(mesh);
			}
			
		private:

			Mesh() = default;

			String name;

			Vector<Vertex> vertices;
			Vector<uint32> indices;

			ComPtr<ID3D11Buffer> vertexBuffer;
			ComPtr<ID3D11Buffer> indexBuffer;

		};
	}
}