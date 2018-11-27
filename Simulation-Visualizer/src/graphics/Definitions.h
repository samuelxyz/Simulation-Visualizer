#pragma once

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

#include <array>
#include <vector>

namespace graphics {

	static constexpr float WINDOW_WIDTH = 1200.0f,
		WINDOW_HEIGHT = 800.0f;

	struct ColoredVertex
	{
		glm::vec4 color;
		glm::vec3 position;
	};

	typedef std::array<ColoredVertex, 3> Triangle;
	typedef std::array<ColoredVertex, 4> Quad;
	typedef std::vector<ColoredVertex> CenteredPoly; // first vertex is center
}