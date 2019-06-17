#include "core/stdafx.h"
#include "core/Definitions.h"
#include "VisualSphere.h"

namespace graphics {

	VisualSphere::VisualSphere(glm::vec3 position, glm::quat orientation, 
		float radius, Style style, glm::vec4 color)
		: VisualEntity(position, orientation, style, color), radius(radius)
	{
	}

	VisualSphere::~VisualSphere()
	{
	}

	void VisualSphere::render(graphics::Renderer& renderer) const
	{
		// TODO

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

		// construct sphere vertices in local frame

		// poles
		graphics::ColoredVertex northPole { colorPlusZ,	 toWorldFrame(glm::vec3(0.0f, 0.0f,  radius)) };
		graphics::ColoredVertex southPole { colorMinusZ, toWorldFrame(glm::vec3(0.0f, 0.0f, -radius), true) };

		// decide mesh resolution
		int numRings = static_cast<int>(std::max(10 * radius, 6.0f )); // ~number of latitude lines
		int numNodes = static_cast<int>(std::max(30 * radius, 12.0f)); // ~number of longitude lines

		// this is to calculate stuff that's the same across every ring, but differs across nodes on each ring
		struct RepeatedNodeData {
			float longitude;
			glm::vec4 colorFactor;
		};

		std::vector<RepeatedNodeData> repeatedNodeData;
		for (int n = 0; n < numNodes; ++n)
		{
			// space out nodes in [0, 2pi)
			// angle = 2pi * n/numNodes
			float longitude = core::TWO_PI * n / numNodes;

			glm::vec4 colorFactor;
			if (longitude < core::HALF_PI)
				colorFactor = glm::mix(colorPlusX, colorPlusY, longitude/core::HALF_PI);
			else if (longitude < core::PI)
				colorFactor = glm::mix(colorPlusY, colorMinusX, longitude/core::HALF_PI - 1);
			else if (longitude < core::HALF_PI + core::PI)
				colorFactor = glm::mix(colorMinusX, colorMinusY, longitude/core::HALF_PI - 2);
			else
				colorFactor = glm::mix(colorMinusY, colorPlusX, longitude/core::HALF_PI - 3);
			
			repeatedNodeData.emplace_back(RepeatedNodeData{ longitude, colorFactor });
		}

		// make vertices
		std::vector<graphics::ColoredVertex> vertices;
		for (int r = 0; r < numRings; ++r)
		{
			// space out rings from -pi/2 to pi/2 exclusive
			float latitudeNormalized = -2 * ((r+1.0f)/(numRings+1.0f) - 0.5f); // -1 at south pole, 1 at north pole
			float latitude = core::HALF_PI * (latitudeNormalized); // -pi/2 at south pole, pi/2 at north pole
			float z = radius * std::sin(latitude);
			float ringRadius = radius * std::cos(latitude);

			glm::vec4 zColorFactor = glm::mix(colorPlusZ, colorMinusZ, (r+1.0f)/(numRings+1.0f));

			for (int n = 0; n < numNodes; ++n)
			{
				glm::vec3 position(ringRadius * std::cos(repeatedNodeData[n].longitude), 
					ringRadius * std::sin(repeatedNodeData[n].longitude), z);

				vertices.emplace_back(graphics::ColoredVertex { 
					glm::mix(repeatedNodeData[n].colorFactor, zColorFactor, std::abs(latitudeNormalized)),
					toWorldFrame(position, true) });
			}
		}

		// add poles at end of vector
		vertices.emplace_back(northPole); 
		vertices.emplace_back(southPole);
		int northPoleIndex = vertices.size() - 2;
		int southPoleIndex = vertices.size() - 1;

		// do indices
		std::vector<int> indices;

		// north pole fan
		for (int n = 1; n < numNodes; ++n)
		{
			indices.emplace_back(northPoleIndex);
			indices.emplace_back(n-1);
			indices.emplace_back(n);
		}
		indices.emplace_back(northPoleIndex);
		indices.emplace_back(numNodes - 1);
		indices.emplace_back(0);

		// south pole fan
		// row = numRows - 1
		int southRingOffset = (numRings-1)*numNodes;
		for (int n = 1; n < numNodes; ++n)
		{
			indices.emplace_back(southPoleIndex);
			indices.emplace_back(southRingOffset + n-1);
			indices.emplace_back(southRingOffset + n);
		}
		indices.emplace_back(southPoleIndex);
		indices.emplace_back(southRingOffset + numNodes - 1);
		indices.emplace_back(southRingOffset);

		// mesh
		
		for (int r = 1; r < numRings; ++r)
		{
			for (int n = 1; n < numNodes; ++n)
			{
				//      n-1     n
				//       |      |
				// r-1 --D------C-- r-1
				//       |      |
				//       |      |
				//   r --B------A-- r
				//       |      |
				//      n-1     n

				indices.emplace_back(r*numNodes + n);		// A
				indices.emplace_back(r*numNodes + n-1);		// B
				indices.emplace_back((r-1)*numNodes + n);	// C
				indices.emplace_back(r*numNodes + n-1);		// B
				indices.emplace_back((r-1)*numNodes + n);	// C
				indices.emplace_back((r-1)*numNodes + n-1);	// D
			}
			//      last    0
			//       |      |
			// r-1 --D------C-- r-1
			//       |      |
			//       |      |
			//   r --B------A-- r
			//       |      |
			//      last    0

			indices.emplace_back(r*numNodes);					// A
			indices.emplace_back(r*numNodes + numNodes-1);		// B
			indices.emplace_back((r-1)*numNodes);				// C
			indices.emplace_back(r*numNodes + numNodes-1);		// B
			indices.emplace_back((r-1)*numNodes);				// C
			indices.emplace_back((r-1)*numNodes + numNodes-1);	// D
		}

		renderer.submit(vertices, indices);
	}
}