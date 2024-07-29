#pragma once

#include "RenderStar/ECS/Component.hpp"
#include "RenderStar/Math/Transform.hpp"
#include "RenderStar/Util/Core/Map.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Math;
using namespace RenderStar::Render;
using namespace RenderStar::Util;

namespace RenderStar
{
	namespace ECS
	{		
		class GameObject : public std::enable_shared_from_this<GameObject>
		{

		public:
			
			GameObject(const GameObject&) = delete;
			GameObject& operator=(const GameObject&) = delete;

			template <typename T>
			Shared<T> AddComponent(Shared<T> component)
			{
				if (!component)
					return nullptr;

				if (std::is_base_of<Component, T>::value == false)
					return nullptr;

				components[TypeIndex(typeid(T))] = component;

				component->gameObject = shared_from_this();
				component->Initialize();

				return component;
			}

			template <typename T>
			Shared<T> GetComponent()
			{
				if (components.Empty())
					return nullptr;

				if (std::is_base_of<Component, T>::value == false)
					return nullptr;

				if (!components.Contains(TypeIndex(typeid(T))))
					return nullptr;

				return std::dynamic_pointer_cast<T>(components[TypeIndex(typeid(T))]);
			}

			template <typename T>
			bool HasComponent()
			{
				if (components.Empty())
					return false;

				if (std::is_base_of<Component, T>::value == false)
					return false;

				return components.Contains(TypeIndex(typeid(T)));
			}

			template <typename T>
			void SetComponent(Shared<T> component)
			{
				if (!component)
					return;

				if (std::is_base_of<Component, T>::value == false)
					return;

				if (!components.Contains(TypeIndex(typeid(T))))
					return;

				components[TypeIndex(typeid(T))] = component;
				component->gameObject = shared_from_this();
				component->Initialize();
			}

			template <typename T>
			void RemoveComponent()
			{
				if (components.Empty())
					return;

				if (std::is_base_of<Component, T>::value == false)
					return;

				if (!components.Contains(TypeIndex(typeid(T))))
					return;

				components[TypeIndex(typeid(T))]->CleanUp();
				components -= TypeIndex(typeid(T));
			}

			void Update()
			{
				for (auto& [type, component] : components)
					component->Update();
			}

			void Render(Shared<Camera> camera)
			{
				for (auto& [type, component] : components)
					component->Render(camera);
			}

			void SetActive(bool value)
			{
				isActive = value;
			}

			bool IsActive() const
			{
				return isActive;
			}

			String GetName() const
			{
				return name;
			}

			void SetName(const String& value)
			{
				name = value;
			}

			void AddChild(Shared<GameObject> child)
			{
				child->parent = shared_from_this();
				child->GetComponent<Transform>()->SetParent(GetComponent<Transform>());
				children += { child->GetName(), child };
			}
			
			Shared<GameObject> GetChild(const String& name)
			{
				if (!children.Contains(name))
					return nullptr;

				return children[name];
			}

			void RemoveChild(const String& name)
			{
				if (!children.Contains(name))
					return;

				children[name]->parent.reset();
				children[name]->GetComponent<Transform>()->SetParent(nullptr);
				children -= name;
			}

			Shared<GameObject> GetParent()
			{
				return parent.lock();
			}

			void CleanUp()
			{
				for (auto& [type, component] : components)
					component->CleanUp();
			}

			static Shared<GameObject> Create(const String& name)
			{
				class Enabled : public GameObject { };
				Shared<GameObject> result = std::make_shared<Enabled>();

				result->name = name;
				result->AddComponent(Transform::Create());

				return std::move(result);
			}

		private:

			GameObject() = default;

			String name;

			bool isActive = true;

			Weak<GameObject> parent;

			Map<TypeIndex, Shared<Component>> components;
			Map<String, Shared<GameObject>> children;

		};
	}
}