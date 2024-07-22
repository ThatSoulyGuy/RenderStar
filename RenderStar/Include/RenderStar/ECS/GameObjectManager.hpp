#pragma once

#include "RenderStar/ECS/GameObject.hpp"

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
				gameObjects += { gameObject->GetName(), gameObject };

				return gameObject;
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

			void Render()
			{
				for (auto& [name, gameObject] : gameObjects)
				{
					if (!gameObject)
						continue;

					gameObject->Render();
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