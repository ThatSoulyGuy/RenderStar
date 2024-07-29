#pragma once

namespace RenderStar
{
	namespace Math
	{
		class Transform;
	}
}

namespace RenderStar
{
	namespace Render
	{
		class Camera;
	}
}

using namespace RenderStar::Math;
using namespace RenderStar::Render;

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
			virtual void Render(Shared<Camera> camera) { }

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