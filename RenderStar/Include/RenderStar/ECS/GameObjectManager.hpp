#pragma once

#include "RenderStar/ECS/GameObject.hpp"

using namespace RenderStar::Render;

namespace RenderStar
{
	namespace ECS
	{
		class GameObjectManager
		{

		public:

			GameObjectManager(const GameObjectManager&) = delete;
			GameObjectManager& operator=(const GameObjectManager&) = delete;

			Shared<GameObject> Register(Shared<GameObject> gameObject)
			{
				String name = gameObject->GetName();

				gameObjects += { name, std::move(gameObject) };

				return gameObjects[name];
			}

			Shared<GameObject> Get(const String& name)
			{
				if (!gameObjects.Contains(name))
					return nullptr;

				return gameObjects[name];
			}

			void Remove(Shared<GameObject> gameObject)
			{
				if (!gameObject)
					return;

				gameObjects -= gameObject->GetName();
			}

			void Update()
			{
				for (auto& [name, gameObject] : gameObjects)
				{
					if (!gameObject)
						continue;

					gameObject->Update();
				}
			}

			void Render(Shared<Camera> camera)
			{
				for (auto& [name, gameObject] : gameObjects)
				{
					if (!gameObject)
						continue;

					gameObject->Render(camera);
				}
			}

			void CleanUp()
			{
				for (auto& [name, gameObject] : gameObjects)
				{
					if (!gameObject)
						continue;

					gameObject->CleanUp();
				}
			}

			static Shared<GameObjectManager> GetInstance()
			{
				class Enabled : public GameObjectManager { };
				static Shared<GameObjectManager> instance = std::make_shared<Enabled>();

				return instance;
			}

		private:

			GameObjectManager() = default;

			Map<String, Shared<GameObject>> gameObjects;
		};
	}
}