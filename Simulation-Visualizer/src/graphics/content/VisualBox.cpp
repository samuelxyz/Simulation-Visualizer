#include "core/stdafx.h"
#include "VisualBox.h"

namespace graphics {

	VisualBox::VisualBox(glm::vec3 position, glm::quat orientation, 
		float xMin, float xMax, float yMin, float yMax, float zMin, float zMax)
		: VisualEntity(position, orientation),
		xMin(xMin), xMax(xMax), yMin(yMin), yMax(yMax), zMin(zMin), zMax(zMax)
	{
	}

	VisualBox::~VisualBox()
	{
	}

	void VisualBox::render(graphics::Renderer& renderer) const
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

		glm::vec4 colorMinusX, colorPlusX, colorMinusY, colorPlusY, colorMinusZ, colorPlusZ;
		if (style == Style::MULTICOLOR)
		{
			colorMinusX = graphics::COLOR_WHITE;
			colorPlusX  = graphics::COLOR_RED;
			colorMinusY = graphics::COLOR_GREEN;
			colorPlusY  = graphics::COLOR_BLUE;
			colorMinusZ = graphics::COLOR_YELLOW;
			colorPlusZ  = graphics::COLOR_BLACK;
		}
		else // style == Style::SOLID_COLOR
		{
			colorMinusX = color;
			colorPlusX  = color;
			colorMinusY = color;
			colorPlusY  = color;
			colorMinusZ = color;
			colorPlusZ  = color;
		}

		graphics::Quad sides[]
		{
			graphics::Quad {{
				{ colorMinusX, vertices[0] },
				{ colorMinusX, vertices[1] },
				{ colorMinusX, vertices[3] },
				{ colorMinusX, vertices[2] }
			}},
			graphics::Quad {{
				{ colorPlusX, vertices[4] },
				{ colorPlusX, vertices[5] },
				{ colorPlusX, vertices[7] },
				{ colorPlusX, vertices[6] }
			}},
			graphics::Quad {{
				{ colorMinusY, vertices[0] },
				{ colorMinusY, vertices[1] },
				{ colorMinusY, vertices[5] },
				{ colorMinusY, vertices[4] }
			}},
			graphics::Quad {{
				{ colorPlusY, vertices[2] },
				{ colorPlusY, vertices[3] },
				{ colorPlusY, vertices[7] },
				{ colorPlusY, vertices[6] }
			}},
			graphics::Quad {{
				{ colorMinusZ, vertices[0] },
				{ colorMinusZ, vertices[2] },
				{ colorMinusZ, vertices[6] },
				{ colorMinusZ, vertices[4] }
			}},
			graphics::Quad {{
				{ colorPlusZ, vertices[1] },
				{ colorPlusZ, vertices[3] },
				{ colorPlusZ, vertices[7] },
				{ colorPlusZ, vertices[5] }
			}}
		};

		for (int i = 0; i < 6; ++i)
		{
			renderer.submit(sides[i]);
		}
	}



}