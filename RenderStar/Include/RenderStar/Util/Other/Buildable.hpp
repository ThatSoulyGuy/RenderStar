#pragma once

#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Util;

namespace RenderStar
{
	namespace Util
	{
		namespace Other
		{
			template <typename T>
			class Buildable
			{

			public:

				virtual ~Buildable() = default;

				virtual T Build() = 0;

			protected:

				Buildable(const Buildable&) = default;
				Buildable& operator=(const Buildable&) = default;

				Buildable() = default;

			};
		}
	}
}