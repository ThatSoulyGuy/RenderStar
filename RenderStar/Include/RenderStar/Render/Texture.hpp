#pragma once

#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <DirectXTex/DirectXTex.h>
#include "RenderStar/Core/Logger.hpp"
#include "RenderStar/Core/Settings.hpp"
#include "RenderStar/ECS/Component.hpp"
#include "RenderStar/Render/Renderer.hpp"
#include "RenderStar/Util/Core/String.hpp"
#include "RenderStar/Util/General/Formatter.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace DirectX;
using namespace RenderStar::Core;
using namespace RenderStar::ECS;
using namespace RenderStar::Render;
using namespace RenderStar::Util;
using namespace RenderStar::Util::Core;
using namespace RenderStar::Util::General;

namespace RenderStar
{
	namespace Render
	{
		class Texture : public Component
		{

		public:

			Texture(const Texture&) = delete;
			Texture& operator=(const Texture&) = delete;

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

			String GetFullPath() const
			{
				return fullPath;
			}

			D3D11_FILTER GetFilter() const
			{
				return filter;
			}

			D3D11_TEXTURE_ADDRESS_MODE GetAddressMode() const
			{
				return addressMode;
			}

			ComPtr<ID3D11Resource> GetTexture() const
			{
				return texture;
			}	

			ComPtr<ID3D11ShaderResourceView> GetTextureView() const
			{
				return textureView;
			}

			void CleanUp() override
			{
				texture.Reset();
				textureView.Reset();
			}

			static Shared<Texture> Create(const String& localPath, const String& name, D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_MODE addressMode = D3D11_TEXTURE_ADDRESS_WRAP, const String& domain = Settings::GetInstance()->Get<String>("defaultDomain"))
			{
				class Enabled : public Texture { };
				Shared<Texture> texture = std::make_shared<Enabled>();

				texture->name = name;
				texture->localPath = localPath;
				texture->domain = domain;
				texture->filter = filter;
				texture->addressMode = addressMode;
				texture->fullPath = Formatter::Format("Assets/{}/{}", domain, localPath);

				texture->Generate();

				return std::move(texture);
			}

		private:

			void Generate()
			{
				TexMetadata metadata;
				ScratchImage scratchImage;

				HRESULT result = DirectX::LoadFromDDSFile(fullPath, DDS_FLAGS_NONE, &metadata, scratchImage);

				if (FAILED(result)) 
				{
					Logger_ThrowException("Failed to load DDS texture.", true);
					return;
				}

				result = DirectX::CreateTexture(Renderer::GetInstance()->GetDevice().Get(), scratchImage.GetImages(), scratchImage.GetImageCount(), metadata, texture.ReleaseAndGetAddressOf());

				if (FAILED(result)) 
				{
					Logger_ThrowException("Failed to create texture from DDS.", true);
					return;
				}

				D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescription = {};

				shaderResourceViewDescription.Format = metadata.format;
				shaderResourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				shaderResourceViewDescription.Texture2D.MostDetailedMip = 0;
				shaderResourceViewDescription.Texture2D.MipLevels = metadata.mipLevels;

				result = Renderer::GetInstance()->GetDevice()->CreateShaderResourceView(texture.Get(), &shaderResourceViewDescription, textureView.ReleaseAndGetAddressOf());

				if (FAILED(result)) 
					Logger_ThrowException("Failed to create shader resource view from texture.", true);
			}

			Texture() = default;

			String name;
			String localPath;
			String domain;
			String fullPath;

			D3D11_FILTER filter;
			D3D11_TEXTURE_ADDRESS_MODE addressMode;

			ComPtr<ID3D11Resource> texture;
			ComPtr<ID3D11ShaderResourceView> textureView;
		};
	}
}