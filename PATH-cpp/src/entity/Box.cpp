#include "Box.h"

namespace entity {

	Box::Box()
		: Box(glm::vec3(0.0f), glm::angleAxis(0.0f, glm::vec3(1.0f, 0.0f, 0.0f)))
	{
	}

	Box::Box(glm::vec3 position, glm::quat orientation)
		: Box(position, orientation, glm::vec3(0.0f), glm::angleAxis(0.0f, glm::vec3(1.0f, 0.0f, 0.0f)))
	{
	}

	Box::Box(glm::vec3 position, glm::quat orientation, glm::vec3 velocity, glm::quat angVel)
		: Box(position, orientation, velocity, angVel, 1.0f, glm::mat3(1.0f))
	{
	}

	Box::Box(glm::vec3 position, glm::quat orientation, glm::vec3 velocity, glm::quat angVel, float mass, glm::mat3 rotInertia)
		: Box(position, orientation, velocity, angVel, mass, rotInertia, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f)
	{
	}

	Box::Box(glm::vec3 position, glm::quat orientation, glm::vec3 velocity, glm::quat angVel,
		float mass, glm::mat3 rotInertia,
		float xMin, float xMax, float yMin, float yMax, float zMin, float zMax)
		: Entity(position, orientation, velocity, angVel, mass, rotInertia),
		xMin(xMin), xMax(xMax), yMin(yMin), yMax(yMax), zMin(zMin), zMax(zMax)
	{
	}

	Box::~Box()
	{
	}

	void Box::render(graphics::Renderer& renderer) const
	{
		glm::vec3 vertices[]
		{
			toWorldFrame(glm::vec3{ xMin, yMin, zMin }), // 0
			toWorldFrame(glm::vec3{ xMin, yMin, zMax }), // 1
			toWorldFrame(glm::vec3{ xMin, yMax, zMin }), // 2
			toWorldFrame(glm::vec3{ xMin, yMax, zMax }), // 3
			toWorldFrame(glm::vec3{ xMax, yMin, zMin }), // 4
			toWorldFrame(glm::vec3{ xMax, yMin, zMax }), // 5
			toWorldFrame(glm::vec3{ xMax, yMax, zMin }), // 6
			toWorldFrame(glm::vec3{ xMax, yMax, zMax }), // 7
		};

		glm::vec4 white { 1.0f, 1.0f, 1.0f, 1.0f },
			red		{ 1.0f, 0.0f, 0.0f, 1.0f },
			green	{ 0.0f, 1.0f, 0.0f, 1.0f },
			blue	{ 0.0f, 0.0f, 1.0f, 1.0f },
			yellow	{ 1.0f, 1.0f, 0.0f, 1.0f },
			black	{ 0.0f, 0.0f, 0.0f, 1.0f };

		graphics::Quad sides[]
		{
			graphics::Quad {{
				{ white, vertices[0] },
				{ white, vertices[1] },
				{ white, vertices[3] },
				{ white, vertices[2] }
			}},
			graphics::Quad {{
				{ red, vertices[4] },
				{ red, vertices[5] },
				{ red, vertices[7] },
				{ red, vertices[6] }
			}},
			graphics::Quad {{
				{ green, vertices[0] },
				{ green, vertices[1] },
				{ green, vertices[5] },
				{ green, vertices[4] }
			}},
			graphics::Quad {{
				{ blue, vertices[2] },
				{ blue, vertices[3] },
				{ blue, vertices[7] },
				{ blue, vertices[6] }
			}},
			graphics::Quad {{
				{ yellow, vertices[0] },
				{ yellow, vertices[2] },
				{ yellow, vertices[6] },
				{ yellow, vertices[4] }
			}},
			graphics::Quad {{
				{ black, vertices[1] },
				{ black, vertices[3] },
				{ black, vertices[7] },
				{ black, vertices[5] }
			}}
		};

		for (int i = 0; i < 6; ++i)
		{
			renderer.submit(sides[i]);
		}

	}
}