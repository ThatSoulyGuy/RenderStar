#pragma once

#include "RenderStar/Math/Transform.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Math;
using namespace RenderStar::Util;

namespace RenderStar
{
	namespace Entity
	{
		class Entity
		{

		public:

			Entity(const Entity&) = delete;
			Entity& operator=(const Entity&) = delete;

			Shared<Transform> GetTransform() const
			{
				return transform;
			}

			static Shared<Entity> Create()
			{
				class Enabled : public Entity { };
				Shared<Entity> result = std::make_shared<Enabled>();

				result->transform = Transform::Create();

				return std::move(result);
			}

		protected:

			Entity() = default;

			Shared<Transform> transform;

		};
	}
}