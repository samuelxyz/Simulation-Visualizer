#include "core/stdafx.h"
#include "VisualCylinder.h"

namespace graphics {

	VisualCylinder::VisualCylinder(glm::vec3 position, glm::quat orientation,
		float radius, float height, Style style, glm::vec4 color, float shadeFactor)
		: VisualEntity(position, orientation, style, color, shadeFactor), radius(radius), height(height)
	{
	}

	VisualCylinder::~VisualCylinder()
	{
	}

	// TODO
	void graphics::VisualCylinder::render(graphics::Renderer & renderer) const
	{
		glm::vec4 colorMinusX, colorPlusX, colorMinusY, colorPlusY, colorMinusZ, colorPlusZ;
		if (style == Style::MULTICOLOR)
		{
			colorMinusX = graphics::COLOR_CYAN;
			colorPlusX = graphics::COLOR_RED;
			colorMinusY = graphics::COLOR_MAGENTA;
			colorPlusY = graphics::COLOR_GREEN;
			colorMinusZ = graphics::COLOR_YELLOW;
			colorPlusZ = graphics::COLOR_BLUE;
		}
		else // style == Style::SOLID_COLOR
		{
			colorMinusX = color;
			colorPlusX = color;
			colorMinusY = color;
			colorPlusY = color;
			colorMinusZ = color;
			colorPlusZ = color;
		}

		// make two faces in local frame
		int numNodes = static_cast<int>(std::max(30 * radius, 12.0f));
		float zTop = height * 0.5f;

		// apply shading
		glm::mat3 orient = glm::toMat3(orientation);

		// within range [-1,1]
		// for cylinder curve
		float* multipliers = new float[numNodes];
		// for faces
		float multTop = (orient * core::VECTOR_UP).z;
		// multBottom would be equivalent to -multTop

		colorPlusZ = glm::mix(colorPlusZ, COLOR_BLACK,
			((std::cos(core::mapRange(multTop, -1.0f, 1.0f, 0.0f, core::PI)) + 1.0f)/2.0f)
			* shadeFactor);

		colorMinusZ = glm::mix(colorMinusZ, COLOR_BLACK,
			((std::cos(core::mapRange(-multTop, -1.0f, 1.0f, 0.0f, core::PI)) + 1.0f)/2.0f)
			* shadeFactor);

		graphics::CenteredPoly topFace, bottomFace;
		topFace.   emplace_back( colorPlusZ,  toWorldFrame(glm::vec3(0.0f, 0.0f,  zTop)) );
		bottomFace.emplace_back( colorMinusZ, toWorldFrame(glm::vec3(0.0f, 0.0f, -zTop), true) );
		for (int i = 0; i < numNodes; ++i)
		{
			float x = radius * std::cosf(core::TWO_PI * i/numNodes);
			float y = radius * std::sinf(core::TWO_PI * i/numNodes);

			topFace.   emplace_back( colorPlusZ,  toWorldFrame(glm::vec3(x, y,  zTop), true) );
			bottomFace.emplace_back( colorMinusZ, toWorldFrame(glm::vec3(x, y, -zTop), true) );

			multipliers[i] = (orient * glm::vec3(x, y, 0.0f)).z;
		}
		// complete the circles
		topFace.push_back(topFace[1]);
		bottomFace.push_back(bottomFace[1]);

		renderer.submit(topFace);
		renderer.submit(bottomFace);

		// mesh for sides
		std::vector<graphics::ColoredVertex> vertices;
		std::vector<int> indices;

		for (int i = 0; i < numNodes; ++i)
		{
			float longitude = core::TWO_PI * i / numNodes;

			glm::vec4 color;
			if (longitude < core::HALF_PI)
				color = glm::mix(colorPlusX, colorPlusY, longitude/core::HALF_PI);
			else if (longitude < core::PI)
				color = glm::mix(colorPlusY, colorMinusX, longitude/core::HALF_PI - 1);
			else if (longitude < core::HALF_PI + core::PI)
				color = glm::mix(colorMinusX, colorMinusY, longitude/core::HALF_PI - 2);
			else
				color = glm::mix(colorMinusY, colorPlusX, longitude/core::HALF_PI - 3);

			// apply shading
			color = glm::mix(color, COLOR_BLACK,
				((std::cos(core::mapRange(multipliers[i], -1.0f, 1.0f, 0.0f, core::PI)) + 1.0f)/2.0f)
				* shadeFactor);

			vertices.emplace_back(color, topFace[i+1].position);
			vertices.emplace_back(color, bottomFace[i+1].position);
		}

		// 2i-2    2i
		// --A------C--
		//   |      |
		//   |      |
		// --B------D--
		// 2i-1    2i+1
		for (int i = 1; i < numNodes; ++i)
		{
			indices.push_back(2*i - 2); // A
			indices.push_back(2*i - 1); // B
			indices.push_back(2*i);     // C
			indices.push_back(2*i - 1); // B
			indices.push_back(2*i);     // C
			indices.push_back(2*i + 1); // D
		}

		// last panel
		int last = 2 * numNodes - 1; // == vertices.size() - 1
		indices.push_back(last - 1);
		indices.push_back(last);
		indices.push_back(0);
		indices.push_back(last);
		indices.push_back(0);
		indices.push_back(1);

		renderer.submit(vertices, indices);
	}
}