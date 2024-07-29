#pragma once

namespace RenderStar
{
	namespace Math
	{
		class Transform;
	}
}

using namespace RenderStar::Math;

namespace RenderStar
{
	namespace ECS
	{		
		class GameObject;
		
		class Component
		{

		public:

			Component& operator=(const Component&) = delete;

			virtual void Initialize() { }

			virtual void Update() { }
			virtual void Render() { }

			virtual void CleanUp() { }

			static Shared<Component> Create()
			{
				class Enabled : public Component { };
				Shared<Component> result = std::make_shared<Enabled>();

				return result;
			}

			Shared<GameObject> gameObject;
		};
	}
}