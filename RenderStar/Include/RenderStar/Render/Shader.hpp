#pragma once

#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include "RenderStar/Core/Logger.hpp"
#include "RenderStar/Core/Settings.hpp"
#include "RenderStar/Render/Renderer.hpp"
#include "RenderStar/Render/Vertex.hpp"
#include "RenderStar/Util/FileHelper.hpp"
#include "RenderStar/Util/Formatter.hpp"
#include "RenderStar/Util/String.hpp"
#include "RenderStar/Util/WString.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Core;
using namespace RenderStar::Util;

#define CompileAndCreateShader(device, path, target, shader) \
{ \
	if (path.IsEmpty()) \
		return; \
 \
	ComPtr<ID3DBlob> shaderBlob; \
	ComPtr<ID3DBlob> errorBlob; \
 \
	HRESULT result = D3DCompileFromFile(WString(path), nullptr, nullptr, "Main", target, 0, 0, &shaderBlob, &errorBlob); \
 \
	if (FAILED(result)) \
	{ \
		if (errorBlob) \
			throw std::runtime_error(static_cast<char*>(errorBlob->GetBufferPointer())); \
 \
		throw std::runtime_error("Shader compilation failed."); \
	} \
 \
	if (target == "vs_5_0") { \
		result = device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, reinterpret_cast<ID3D11VertexShader**>(shader)); \
		vertexBlob = shaderBlob; \
	} \
	else if (target == "ps_5_0") \
		result = device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, reinterpret_cast<ID3D11PixelShader**>(shader)); \
	else if (target == "cs_5_0") \
		result = device->CreateComputeShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, reinterpret_cast<ID3D11ComputeShader**>(shader)); \
	else if (target == "ds_5_0") \
		result = device->CreateDomainShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, reinterpret_cast<ID3D11DomainShader**>(shader)); \
	else if (target == "gs_5_0") \
		result = device->CreateGeometryShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, reinterpret_cast<ID3D11GeometryShader**>(shader)); \
	else if (target == "hs_5_0") \
		result = device->CreateHullShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, reinterpret_cast<ID3D11HullShader**>(shader)); \
 \
	if (FAILED(result)) \
		throw std::runtime_error("Failed to create shader."); \
} 

namespace RenderStar
{
	namespace Render
	{
		enum class ShaderType
		{
			VERTEX,
			PIXEL,
			COMPUTE,
			DOMAIN,
			GEOMETRY,
			HULL
		};

		class Shader
		{

		public:

			Shader(const Shader&) = delete;
			Shader& operator=(const Shader&) = delete;

			void Bind() 
			{
				ComPtr<ID3D11DeviceContext> context = Renderer::GetInstance()->GetContext();

				if (vertexShader) 
					context->VSSetShader(vertexShader.Get(), nullptr, 0);

				if (pixelShader) 
					context->PSSetShader(pixelShader.Get(), nullptr, 0);

				if (computeShader) 
					context->CSSetShader(computeShader.Get(), nullptr, 0);

				if (domainShader) 
					context->DSSetShader(domainShader.Get(), nullptr, 0);

				if (geometryShader) 
					context->GSSetShader(geometryShader.Get(), nullptr, 0);

				if (hullShader) 
					context->HSSetShader(hullShader.Get(), nullptr, 0);

				context->IASetInputLayout(inputLayout.Get());
			}

			template <typename T>
			void SetConstantBuffer(UINT slot, T data, ShaderType shaderType)
			{
				ComPtr<ID3D11DeviceContext> context = Renderer::GetInstance()->GetContext();

				if (!constantBuffers.Contains(slot))
				{
					D3D11_BUFFER_DESC bufferDescription = {};

					bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
					bufferDescription.ByteWidth = sizeof(T);
					bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
					bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

					HRESULT result = Renderer::GetInstance()->GetDevice()->CreateBuffer(&bufferDescription, nullptr, &constantBuffers[slot]);

					if (FAILED(result))
					{
						Logger_ThrowException("Failed to create constant buffer.", false);
						return;
					}
				}

				D3D11_MAPPED_SUBRESOURCE mappedResource;
				HRESULT result = context->Map(constantBuffers[slot].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

				if (FAILED(result))
				{
					Logger_ThrowException("Failed to map constant buffer.", false);
					return;
				}

				memcpy(mappedResource.pData, &data, sizeof(T));
				context->Unmap(constantBuffers[slot].Get(), 0);

				SetConstantBuffer(slot, constantBuffers[slot], shaderType);
			}

			void SetConstantBuffer(UINT slot, ComPtr<ID3D11Buffer> constantBuffer, ShaderType shaderType)
			{
				ComPtr<ID3D11DeviceContext> context = Renderer::GetInstance()->GetContext();

				switch (shaderType)
				{

				case ShaderType::VERTEX:
					context->VSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf());
					break;

				case ShaderType::PIXEL:
					context->PSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf());
					break;

				case ShaderType::COMPUTE:
					context->CSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf());
					break;

				case ShaderType::DOMAIN:
					context->DSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf());
					break;

				case ShaderType::GEOMETRY:
					context->GSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf());
					break;

				case ShaderType::HULL:
					context->HSSetConstantBuffers(slot, 1, constantBuffer.GetAddressOf());
					break;

				default:
					throw std::invalid_argument("Unknown shader type");
				}
			}

			void SetSamplerState(UINT slot, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE mode, ShaderType shaderType)
			{
				ComPtr<ID3D11SamplerState> samplerState;

				D3D11_SAMPLER_DESC samplerDescription = {};

				samplerDescription.Filter = filter;

				samplerDescription.AddressU = mode;
				samplerDescription.AddressV = mode;
				samplerDescription.AddressW = mode;

				samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
				samplerDescription.MinLOD = 0;
				samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

				HRESULT result = Renderer::GetInstance()->GetDevice()->CreateSamplerState(&samplerDescription, samplerState.GetAddressOf());

				if (FAILED(result))
				{
					Logger_ThrowException("Failed to create sampler state.", false);
					return;
				}

				SetSamplerState(slot, samplerState, shaderType);
			}

			void SetSamplerState(UINT slot, ComPtr<ID3D11SamplerState> samplerState, ShaderType shaderType)
			{
				ComPtr<ID3D11DeviceContext> context = Renderer::GetInstance()->GetContext();

				switch (shaderType)
				{

				case ShaderType::VERTEX:
					context->VSSetSamplers(slot, 1, samplerState.GetAddressOf());
					break;

				case ShaderType::PIXEL:
					context->PSSetSamplers(slot, 1, samplerState.GetAddressOf());
					break;

				case ShaderType::COMPUTE:
					context->CSSetSamplers(slot, 1, samplerState.GetAddressOf());
					break;

				case ShaderType::DOMAIN:
					context->DSSetSamplers(slot, 1, samplerState.GetAddressOf());
					break;

				case ShaderType::GEOMETRY:
					context->GSSetSamplers(slot, 1, samplerState.GetAddressOf());
					break;

				case ShaderType::HULL:
					context->HSSetSamplers(slot, 1, samplerState.GetAddressOf());
					break;

				default:
					throw std::invalid_argument("Unknown shader type");
				}
			}

			String GetName() const
			{
				return name;
			}

			String GetLocalPath() const
			{
				return localPath;
			}

			String GetDomain() const
			{
				return domain;
			}

			Map<String, String> GetPaths() const
			{
				Map<String, String> paths;

				paths["vertex"] = vertexPath;
				paths["pixel"] = pixelPath;
				paths["compute"] = computePath;
				paths["domain"] = domainPath;
				paths["geometry"] = geometryPath;
				paths["hull"] = hullPath;

				return paths;
			}

			void CleanUp()
			{
				if (vertexShader)
					vertexShader.Reset();

				if (pixelShader)
					pixelShader.Reset();

				if (computeShader)
					computeShader.Reset();

				if (domainShader)
					domainShader.Reset();

				if (geometryShader)
					geometryShader.Reset();

				if (hullShader)
					hullShader.Reset();
			}

			static Shared<Shader> Create(const String& localPath, const String& name, const String& domain = Settings::GetInstance()->Get<String>("defaultDomain"))
			{
				Shared<Shader> shader = Shared<Shader>(new Shader());

				shader->name = name;
				shader->localPath = localPath;
				shader->domain = domain;

				shader->vertexPath = Formatter::Format("Assets/{}/{}Vertex.hlsl", domain, localPath);
				shader->pixelPath = Formatter::Format("Assets/{}/{}Pixel.hlsl", domain, localPath);
				shader->computePath = Formatter::Format("Assets/{}/{}Compute.hlsl", domain, localPath);
				shader->domainPath = Formatter::Format("Assets/{}/{}Domain.hlsl", domain, localPath);
				shader->geometryPath = Formatter::Format("Assets/{}/{}Geometry.hlsl", domain, localPath);
				shader->hullPath = Formatter::Format("Assets/{}/{}Hull.hlsl", domain, localPath);

				shader->Generate();

				return shader;
			}

		private:

			Shader() = default;

			void Generate()
			{
				ComPtr<ID3D11Device> device = Renderer::GetInstance()->GetDevice();

				if (FileHelper::GetInstance()->FileExists(vertexPath))
					CompileAndCreateShader(device, vertexPath, "vs_5_0", vertexShader.ReleaseAndGetAddressOf());

				if (FileHelper::GetInstance()->FileExists(pixelPath))
					CompileAndCreateShader(device, pixelPath, "ps_5_0", pixelShader.ReleaseAndGetAddressOf());

				if (FileHelper::GetInstance()->FileExists(computePath))
					CompileAndCreateShader(device, computePath, "cs_5_0", computeShader.ReleaseAndGetAddressOf());

				if (FileHelper::GetInstance()->FileExists(domainPath))
					CompileAndCreateShader(device, domainPath, "ds_5_0", domainShader.ReleaseAndGetAddressOf());
				
				if (FileHelper::GetInstance()->FileExists(geometryPath))
					CompileAndCreateShader(device, geometryPath, "gs_5_0", geometryShader.ReleaseAndGetAddressOf());

				if (FileHelper::GetInstance()->FileExists(hullPath))
					CompileAndCreateShader(device, hullPath, "hs_5_0", hullShader.ReleaseAndGetAddressOf());

				if (vertexShader && pixelShader)
				{
					Array<D3D11_INPUT_ELEMENT_DESC, 4> inputElementDescription = Vertex::GetInputElementLayout();

					HRESULT result = device->CreateInputLayout(inputElementDescription, inputElementDescription.Length(), vertexBlob->GetBufferPointer(), vertexBlob->GetBufferSize(), inputLayout.ReleaseAndGetAddressOf());

					if (FAILED(result))
						throw std::runtime_error("Failed to create input layout.");
				}
			}

			String name;
			String localPath;
			String domain;

			String vertexPath, pixelPath, computePath, domainPath, geometryPath, hullPath;

			ComPtr<ID3DBlob> vertexBlob;

			ComPtr<ID3D11VertexShader> vertexShader;
			ComPtr<ID3D11PixelShader> pixelShader;
			ComPtr<ID3D11ComputeShader> computeShader;
			ComPtr<ID3D11DomainShader> domainShader;
			ComPtr<ID3D11GeometryShader> geometryShader;
			ComPtr<ID3D11HullShader> hullShader;

			ComPtr<ID3D11InputLayout> inputLayout;

			Map<uint32, ComPtr<ID3D11Buffer>> constantBuffers;
		};
	}
}