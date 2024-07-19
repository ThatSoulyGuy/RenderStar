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
		};
	}
}