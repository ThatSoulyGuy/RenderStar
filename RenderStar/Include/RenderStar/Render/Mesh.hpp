#pragma once

#include "RenderStar/Core/Logger.hpp"
#include "RenderStar/Render/Renderer.hpp"
#include "RenderStar/Render/Shader.hpp"
#include "RenderStar/Render/Vertex.hpp"
#include "RenderStar/Util/String.hpp"
#include "RenderStar/Util/Vector.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Core;
using namespace RenderStar::Util;

namespace RenderStar
{
	namespace Render
	{
		class Mesh
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

			void Render()
			{
				ComPtr<ID3D11DeviceContext> context = Renderer::GetInstance()->GetContext();

				UINT stride = sizeof(Vertex);
				UINT offset = 0;

				shader->Bind();

				context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
				context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
				context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				context->DrawIndexed(indices.Length(), 0, 0);
			}

			void CleanUp()
			{
				vertexBuffer.Reset();
				indexBuffer.Reset();
			}

			static Shared<Mesh> Create(const String& name, const Shared<Shader>& shader, const Vector<Vertex>& vertices, const Vector<uint32>& indices)
			{
				class Enabled : public Mesh { };
				Shared<Mesh> mesh = std::make_shared<Enabled>();

				mesh->name = name;
				mesh->shader = shader;
				mesh->vertices = vertices;
				mesh->indices = indices;

				return mesh;
			}

		private:

			Mesh() = default;

			String name;

			Shared<Shader> shader;	

			Vector<Vertex> vertices;
			Vector<uint32> indices;

			ComPtr<ID3D11Buffer> vertexBuffer;
			ComPtr<ID3D11Buffer> indexBuffer;

		};
	}
}