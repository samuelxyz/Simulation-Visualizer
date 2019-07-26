#include "core/stdafx.h"
#include "core/Definitions.h"
#include "VisualBox.h"

namespace graphics {

	VisualBox::VisualBox(const glm::vec3& position, const glm::quat& orientation, 
		float xMin, float xMax, float yMin, float yMax, float zMin, float zMax,
		Style style, const glm::vec4& color, float shadeFactor)
		: VisualEntity(position, orientation, style, color, shadeFactor),
		xMin(xMin), xMax(xMax), yMin(yMin), yMax(yMax), zMin(zMin), zMax(zMax)
	{
	}

	VisualBox::VisualBox(const glm::vec3& start, const glm::vec3& length, float thickness, 
		const glm::vec3& up, Style style, const glm::vec4& color, float shadeFactor)
		: VisualEntity(start, core::quatLookAt(length, up), style, color, shadeFactor),
		xMin(-thickness), xMax(thickness + glm::length(length)), 
		yMin(-thickness), yMax(thickness), zMin(-thickness), zMax(thickness)
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
			toWorldFrame(glm::vec3{ xMin, yMin, zMax }, true), // 1
			toWorldFrame(glm::vec3{ xMin, yMax, zMin }, true), // 2
			toWorldFrame(glm::vec3{ xMin, yMax, zMax }, true), // 3
			toWorldFrame(glm::vec3{ xMax, yMin, zMin }, true), // 4
			toWorldFrame(glm::vec3{ xMax, yMin, zMax }, true), // 5
			toWorldFrame(glm::vec3{ xMax, yMax, zMin }, true), // 6
			toWorldFrame(glm::vec3{ xMax, yMax, zMax }, true), // 7
		};

		glm::vec4 colors[6];
		
		// names
		glm::vec4 
			& colorMinusX	= colors[0], 
			& colorPlusX	= colors[1], 
			& colorMinusY	= colors[2], 
			& colorPlusY	= colors[3], 
			& colorMinusZ	= colors[4], 
			& colorPlusZ	= colors[5];

		if (style == Style::MULTICOLOR)
		{
			colorMinusX = graphics::COLOR_CYAN;
			colorPlusX  = graphics::COLOR_RED;
			colorMinusY = graphics::COLOR_MAGENTA;
			colorPlusY  = graphics::COLOR_GREEN;
			colorMinusZ = graphics::COLOR_YELLOW;
			colorPlusZ  = graphics::COLOR_BLUE;
		}
		else // if (style == Style::SOLID_COLOR)
		{
			for (glm::vec4& c : colors)
				c = color;
		}

		if (shadeFactor != 0.0f)
		{
			glm::mat3 orientMat = glm::toMat3(orientation);

			// should be within [-1, 1]
			float multipliers[]
			{
				(orientMat * glm::vec3{ -1.0f,  0.0f,  0.0f }).z,
				(orientMat * glm::vec3{  1.0f,  0.0f,  0.0f }).z,
				(orientMat * glm::vec3{  0.0f, -1.0f,  0.0f }).z,
				(orientMat * glm::vec3{  0.0f,  1.0f,  0.0f }).z,
				(orientMat * glm::vec3{  0.0f,  0.0f, -1.0f }).z,
				(orientMat * glm::vec3{  0.0f,  0.0f,  1.0f }).z
			};

			for (int i = 0; i < 6; ++i)
			{
				colors[i] = glm::mix(colors[i], COLOR_BLACK,
					((std::cos(core::mapRange(multipliers[i], -1.0f, 1.0f, 0.0f, core::PI)) + 1.0f)/2.0f)
					* shadeFactor);
			}
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