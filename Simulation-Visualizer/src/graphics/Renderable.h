#pragma once
#include "graphics/Renderer.h"

namespace graphics {
	class Renderable
	{
	public:
		virtual void render(Renderer& renderer) const = 0;
	};
}