#pragma once

#define SILENCE_QUATERNION

#include "RenderStar/Core/Logger.hpp"
#include "RenderStar/ECS/GameObjectManager.hpp"
#include "RenderStar/Render/Mesh.hpp"
#include "RenderStar/Render/Renderer.hpp"
#include "RenderStar/Render/ShaderManager.hpp"
#include "RenderStar/Util/General/CommonVersionFormat.hpp"
#include "RenderStar/Util/Other/XXMLParser.hpp"

using namespace RenderStar::Core;
using namespace RenderStar::Render;

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

				default:

					return DefWindowProc(handle, message, wParam, lParam);
				}
			});

			Logger_WriteConsole("Pre-initializing engine...", LogLevel::INFORMATION);
		}

		void Initialize()
		{
			Logger_WriteConsole("Initializing engine...", LogLevel::INFORMATION);

			Renderer::GetInstance()->Initialize();

			ShaderManager::GetInstance()->Register(Shader::Create("Shader/Default", "default"));

			Shared<GameObject> mesh = GameObjectManager::GetInstance()->Register(GameObject::Create("Mesh"));
				
			mesh->AddComponent(ShaderManager::GetInstance()->Get("default"));
			mesh->AddComponent(Mesh::Create("default",
			{
				{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
				{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
				{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
				{ { -0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } }
			}, 
			{ 
				0, 2, 1,
				2, 0, 3
			}));

			mesh->GetComponent<Mesh>()->Generate();
		}

		void Update()
		{

			GameObjectManager::GetInstance()->Update();
		}

		void Render()
		{
			Renderer::GetInstance()->PreRender();

			GameObjectManager::GetInstance()->Render();

			Renderer::GetInstance()->PostRender();
		}

		void CleanUp()
		{
			Logger_WriteConsole("Cleaning up engine...", LogLevel::INFORMATION);

			GameObjectManager::GetInstance()->CleanUp();
			ShaderManager::GetInstance()->CleanUp();
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
	};
}