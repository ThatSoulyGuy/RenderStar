#pragma once

#include <d3d11_4.h>
#include <dxgi1_6.h>
#include "RenderStar/Math/Vector2.hpp"
#include "RenderStar/Math/Vector3.hpp"
#include "RenderStar/Util/Core/Array.hpp"
#include "RenderStar/Util/Typedefs.hpp"

using namespace RenderStar::Math;
using namespace RenderStar::Util;

namespace RenderStar
{
	namespace Render
	{
		struct Vertex
		{
			Vector3f position;
			Vector3f color;
			Vector3f normal;
			Vector2f textureCoordinates;

			static Array<D3D11_INPUT_ELEMENT_DESC, 4> GetInputElementLayout()
			{
				return
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
					{ "TEXTURECOORDINATES", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
				};
			}
		};
	}
}