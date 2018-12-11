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

	static const glm::vec4
		COLOR_WHITE	{ 1.0f, 1.0f, 1.0f, 1.0f },
		COLOR_RED	{ 1.0f, 0.0f, 0.0f, 1.0f },
		COLOR_GREEN	{ 0.0f, 1.0f, 0.0f, 1.0f },
		COLOR_BLUE	{ 0.0f, 0.0f, 1.0f, 1.0f },
		COLOR_YELLOW{ 1.0f, 1.0f, 0.0f, 1.0f },
		COLOR_BLACK	{ 0.0f, 0.0f, 0.0f, 1.0f };

	typedef std::array<ColoredVertex, 3> Triangle;
	typedef std::array<ColoredVertex, 4> Quad; // clockwise or counterclockwise
	typedef std::vector<ColoredVertex> CenteredPoly; // first vertex is center
}