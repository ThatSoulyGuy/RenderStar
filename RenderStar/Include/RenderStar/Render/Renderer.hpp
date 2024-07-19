#pragma once

#include <d3d11_4.h>
#include <dxgi1_6.h>
#include "RenderStar/Core/Logger.hpp"
#include "RenderStar/Core/Window.hpp"
#include "RenderStar/Math/Vector2.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Core;
using namespace RenderStar::Math;
using namespace RenderStar::Util;

namespace RenderStar
{
	namespace Render
	{
		class Renderer
		{

		public:

			Renderer(const Renderer&) = delete;
			Renderer& operator=(const Renderer&) = delete;

			void Initialize()
			{
				LockGuard<Mutex> lock(mutex);

				Logger_WriteConsole("Initializing renderer...", LogLevel::INFORMATION);

				UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
				creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

				HRESULT result = S_OK;

				result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, nullptr, 0, D3D11_SDK_VERSION, &device, nullptr, &context);

				if (FAILED(result))
				{
					Logger_WriteConsole("Failed to create Direct3D 11 device.", LogLevel::ERROR);
					return;
				}

				DXGI_SWAP_CHAIN_DESC1 swapChainDescription = {};

				swapChainDescription.BufferCount = 3;
				swapChainDescription.Width = 0;
				swapChainDescription.Height = 0;
				swapChainDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				swapChainDescription.SampleDesc.Count = 1;
				swapChainDescription.SampleDesc.Quality = 0;

				IDXGIFactory2* factory = nullptr;
				result = CreateDXGIFactory1(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&factory));

				if (FAILED(result))
				{
					Logger_WriteConsole("Failed to create DXGI factory.", LogLevel::ERROR);
					return;
				}

				result = factory->CreateSwapChainForHwnd(device.Get(), Window::GetInstance()->GetHandle(), &swapChainDescription, nullptr, nullptr, &swapChain);
				factory->Release();

				currentDimensions = Vector2i{ Window::GetInstance()->GetClientDimensions().x, Window::GetInstance()->GetClientDimensions().y };

				if (FAILED(result))
				{
					Logger_WriteConsole("Failed to create swap chain.", LogLevel::ERROR);
					return;
				}

				ID3D11Texture2D* backBuffer = nullptr;
				result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));

				if (FAILED(result))
				{
					Logger_WriteConsole("Failed to get back buffer.", LogLevel::ERROR);
					return;
				}

				result = device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
				backBuffer->Release();

				if (FAILED(result))
				{
					Logger_WriteConsole("Failed to create render target view.", LogLevel::ERROR);
					return;
				}

				context->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);

				D3D11_VIEWPORT viewport = {};

				viewport.Width = static_cast<FLOAT>(currentDimensions.x);
				viewport.Height = static_cast<FLOAT>(currentDimensions.y);
				viewport.MinDepth = 0.0f;
				viewport.MaxDepth = 1.0f;
				viewport.TopLeftX = 0;
				viewport.TopLeftY = 0;

				context->RSSetViewports(1, &viewport);

				isInitialized = true;
			}

			void PreRender() const
			{
				LockGuard<Mutex> lock(mutex);

				context->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);

				FLOAT clearColor[4] = { 0.0f, 0.45f, 0.75f, 1.0f };
				context->ClearRenderTargetView(renderTargetView.Get(), clearColor);
			}

			void PostRender() const
			{
				LockGuard<Mutex> lock(mutex);

				swapChain->Present(1, 0);
			}

			void Resize(const Vector2i& dimensions)
			{
				LockGuard<Mutex> lock(mutex);

				if (!isInitialized)
					return;

				currentDimensions = dimensions;

				if (renderTargetView)
					renderTargetView.Reset();

				swapChain->ResizeBuffers(0, currentDimensions.x, currentDimensions.y, DXGI_FORMAT_UNKNOWN, 0);

				HRESULT result = S_OK;

				ID3D11Texture2D* backBuffer = nullptr;

				result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));

				if (FAILED(result))
				{
					Logger_WriteConsole("Failed to get back buffer.", LogLevel::ERROR);
					return;
				}

				result = device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);

				backBuffer->Release();

				if (FAILED(result))
				{
					Logger_WriteConsole("Failed to create render target view.", LogLevel::ERROR);
					return;
				}

				context->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), nullptr);

				D3D11_VIEWPORT viewport = {};

				viewport.Width = static_cast<FLOAT>(currentDimensions.x);
				viewport.Height = static_cast<FLOAT>(currentDimensions.y);
				viewport.MinDepth = 0.0f;
				viewport.MaxDepth = 1.0f;
				viewport.TopLeftX = 0;
				viewport.TopLeftY = 0;

				context->RSSetViewports(1, &viewport);
			}

			bool IsInitialized() const
			{
				return isInitialized;
			}

			ComPtr<ID3D11Device> GetDevice() const
			{
				return device;
			}

			ComPtr<ID3D11DeviceContext> GetContext() const
			{
				return context;
			}

			ComPtr<IDXGISwapChain1> GetSwapChain() const
			{
				return swapChain;
			}

			ComPtr<ID3D11RenderTargetView> GetRenderTargetView() const
			{
				return renderTargetView;
			}

			void CleanUp()
			{
				if (renderTargetView)
					renderTargetView.Reset();

				if (swapChain)
					swapChain.Reset();

				if (context)
					context.Reset();

				if (device)
					device.Reset();

				Logger_WriteConsole("Renderer cleaned up.", LogLevel::INFORMATION);
			}

			static Shared<Renderer> GetInstance()
			{
				class Enabled : public Renderer { };
				static Shared<Renderer> instance = std::make_shared<Enabled>();

				return instance;
			}

		private:

			Renderer() = default;

			ComPtr<ID3D11Device> device = nullptr;
			ComPtr<ID3D11DeviceContext> context = nullptr;
			ComPtr<IDXGISwapChain1> swapChain = nullptr;
			ComPtr<ID3D11RenderTargetView> renderTargetView = nullptr;

			Vector2i currentDimensions;

			bool isInitialized = false;

			mutable Mutex mutex;
		};
	}
}