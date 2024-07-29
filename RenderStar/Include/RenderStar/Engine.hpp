#pragma once

#define SILENCE_QUATERNION

#include "RenderStar/Core/InputManager.hpp"
#include "RenderStar/Core/Logger.hpp"
#include "RenderStar/ECS/GameObjectManager.hpp"
#include "RenderStar/Entity/Entities/EntityPlayer.hpp"
#include "RenderStar/Render/Mesh.hpp"
#include "RenderStar/Render/Renderer.hpp"
#include "RenderStar/Render/ShaderManager.hpp"
#include "RenderStar/Render/TextureManager.hpp"
#include "RenderStar/Util/General/CommonVersionFormat.hpp"
#include "RenderStar/Util/Other/XXMLParser.hpp"

using namespace RenderStar::Core;
using namespace RenderStar::Render;
using namespace RenderStar::ECS;
using namespace RenderStar::Entity;
using namespace RenderStar::Util::Core;
using namespace RenderStar::Util::General;
using namespace RenderStar::Util::Other;

namespace RenderStar
{
	class Engine
	{

	public:

		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;
		
		void PreInitialize()
		{
			Shared<XXMLParser> settings = XXMLParser::Create("EngineSettings.xxml", "Settings");

			if (!settings->GetGlobalVariables().Contains("ID"))
			{
				Logger_ThrowException("Engine settings file is invalid.", true);
				return;
			}

			if (settings->GetGlobalVariable<std::string>("ID") != std::string("0x00003E91A376E7AB"))
			{
				Logger_ThrowException("Engine settings file has incorrect ID!", true);
				return;
			}

			if (!settings->GetNamespaces()["RenderStar_Default"].Contains("DefaultApplicationTitle"))
			{
				Logger_ThrowException("Engine settings file does not contain an application title!", true);
				return;
			}

			if (!settings->GetNamespaces()["RenderStar_Default"].Contains("DefaultApplicationVersion"))
			{
				Logger_ThrowException("Engine settings file does not contain an application version!", true);
				return;
			}

			if (!settings->GetNamespaces()["RenderStar_Default"].Contains("DefaultWindowDimensions"))
			{
				Logger_ThrowException("Engine settings file does not contain an window size!", true);
				return;
			}

			Settings::GetInstance()->Set<String>("defaultApplicationTitle", settings->GetNamespacedVariable<std::string>("RenderStar_Default", "DefaultApplicationTitle"));
			Settings::GetInstance()->Set<CommonVersionFormat>("defaultApplicationVersion", settings->GetNamespacedVariable<CommonVersionFormat>("RenderStar_Default", "DefaultApplicationVersion"));
			Settings::GetInstance()->Set<Vector2i>("defaultWindowDimensions", settings->GetNamespacedVariable<Vector2i>("RenderStar_Default", "DefaultWindowDimensions"));
			Settings::GetInstance()->Set<WNDPROC>("defaultWindowProcedure", [](HWND handle, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
			{
				switch (message)
				{

				case WM_SIZE:

					if (wParam != SIZE_MINIMIZED)
						Renderer::GetInstance()->Resize({ LOWORD(lParam), HIWORD(lParam) });

					return 0;

				case WM_DESTROY:
					PostQuitMessage(0);
					return 0;

				case WM_KEYDOWN:
					InputManager::GetInstance()->SetKeyState(static_cast<KeyCode>(wParam), KeyState::PRESSED);
					return 0;

				case WM_KEYUP:
					InputManager::GetInstance()->SetKeyState(static_cast<KeyCode>(wParam), KeyState::RELEASED);
					return 0;

				case WM_LBUTTONDOWN:
					InputManager::GetInstance()->SetMouseButtonState(MouseCode::LEFT, MouseState::PRESSED);
					return 0;

				case WM_LBUTTONUP:
					InputManager::GetInstance()->SetMouseButtonState(MouseCode::LEFT, MouseState::RELEASED);
					return 0;

				case WM_RBUTTONDOWN:
					InputManager::GetInstance()->SetMouseButtonState(MouseCode::RIGHT, MouseState::PRESSED);
					return 0;

				case WM_RBUTTONUP:
					InputManager::GetInstance()->SetMouseButtonState(MouseCode::RIGHT, MouseState::RELEASED);
					return 0;

				case WM_MBUTTONDOWN:
					InputManager::GetInstance()->SetMouseButtonState(MouseCode::MIDDLE, MouseState::PRESSED);
					return 0;

				case WM_MBUTTONUP:
					InputManager::GetInstance()->SetMouseButtonState(MouseCode::MIDDLE, MouseState::RELEASED);
					return 0;

				case WM_MOUSEMOVE:
				{
					POINTS points = MAKEPOINTS(lParam);
					InputManager::GetInstance()->SetMousePosition({ points.x, points.y });
				}
				return 0;

				default:

					return DefWindowProc(handle, message, wParam, lParam);
				}
			});

			lastTime = std::chrono::high_resolution_clock::now();
			frameCount = 0;
			fps = 0.0;

			Logger_WriteConsole("Pre-initializing engine...", LogLevel::INFORMATION);
		}

		void Initialize()
		{
			Logger_WriteConsole("Initializing engine...", LogLevel::INFORMATION);

			Shared<Transform> transform = Transform::Create();
			transform->SetRotation(Vector3f(0.0f, 90.0f, 0.0f));

			Renderer::GetInstance()->Initialize();

			ShaderManager::GetInstance()->Register(Shader::Create("Shader/Default", "default"));
			TextureManager::GetInstance()->Register(Texture::Create("Texture/Debug.dds", "debug", D3D11_FILTER_MIN_MAG_MIP_POINT));

			Shared<GameObject> player = GameObjectManager::GetInstance()->Register(GameObject::Create("Player"));
			player->GetComponent<Transform>()->SetPosition({ 0.0f, 0.0f, -10.0f });
			player->AddComponent(IEntity::Create<EntityPlayer>());

			Shared<GameObject> mesh = GameObjectManager::GetInstance()->Register(GameObject::Create("Mesh"));
				
			mesh->AddComponent(ShaderManager::GetInstance()->Get("default"));
			mesh->AddComponent(TextureManager::GetInstance()->Get("debug"));
			mesh->AddComponent(Mesh::Create("default",
			{
				{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
				{ {  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
				{ {  0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
				{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } }
			}, 
			{ 
				2, 1, 0, 
				3, 2, 0
			}));

			mesh->GetComponent<Mesh>()->Generate();
		}

		void Update()
		{
			InputManager::GetInstance()->Update();
			GameObjectManager::GetInstance()->Update();

			frameCount++;
			auto currentTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> elapsed = currentTime - lastTime;

			if (elapsed.count() >= 1.0)
			{
				fps = frameCount / elapsed.count();
				frameCount = 0;
				lastTime = currentTime;

				Logger_WriteConsole("Current FPS: " + std::to_string(fps), LogLevel::INFORMATION);
			}
		}

		void Render()
		{
			Renderer::GetInstance()->PreRender();

			GameObjectManager::GetInstance()->Render(Renderer::GetInstance()->GetCamera());

			Renderer::GetInstance()->PostRender();
		}

		void CleanUp()
		{
			Logger_WriteConsole("Cleaning up engine...", LogLevel::INFORMATION);

			GameObjectManager::GetInstance()->CleanUp();
			ShaderManager::GetInstance()->CleanUp();
			TextureManager::GetInstance()->CleanUp();
			Renderer::GetInstance()->CleanUp();
		}

		static Shared<Engine> GetInstance()
		{
			class Enabled : public Engine { };
			static Shared<Engine> instance = std::make_shared<Enabled>();

			return instance;
		}

	private:

		Engine() = default;

		std::chrono::high_resolution_clock::time_point lastTime;
		unsigned int frameCount;
		double fps;
	};
}