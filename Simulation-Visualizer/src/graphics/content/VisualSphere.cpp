#include "core/stdafx.h"
#include "VisualSphere.h"

namespace graphics {

	VisualSphere::VisualSphere(glm::vec3 position, glm::quat orientation, 
		float radius, Style style, glm::vec4 color, float shadeFactor)
		: VisualEntity(position, orientation, style, color, shadeFactor), radius(radius)
	{
	}

	VisualSphere::~VisualSphere()
	{
	}

	void VisualSphere::render(graphics::Renderer& renderer) const
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

			RepeatedNodeData(float longitude, const glm::vec4& colorFactor)
				: longitude(longitude), colorFactor(colorFactor)
			{}
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
			
			repeatedNodeData.emplace_back(longitude, colorFactor);
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

				vertices.emplace_back(
					glm::mix(repeatedNodeData[n].colorFactor, zColorFactor, std::abs(latitudeNormalized)),
					toWorldFrame(position, true));
			}
		}

		// add poles at end of vector
		vertices.push_back(northPole); 
		vertices.push_back(southPole);
		int northPoleIndex = vertices.size() - 2;
		int southPoleIndex = vertices.size() - 1;

		// apply shading
		if (shadeFactor != 0.0f)
		{
			for (ColoredVertex& cv : vertices)
			{
				// within range [-1, 1]
				float multiplier = (cv.position.z - position.z) / radius;
				cv.color = glm::mix(cv.color, COLOR_BLACK,
					((std::cos(core::mapRange(multiplier, -1.0f, 1.0f, 0.0f, core::PI)) + 1.0f)/2.0f)
					* shadeFactor);
			}
		}

		// do indices
		std::vector<int> indices;

		// north pole fan
		for (int n = 1; n < numNodes; ++n)
		{
			indices.push_back(northPoleIndex);
			indices.push_back(n-1);
			indices.push_back(n);
		}
		indices.push_back(northPoleIndex);
		indices.push_back(numNodes - 1);
		indices.push_back(0);

		// south pole fan
		// row = numRows - 1
		int southRingOffset = (numRings-1)*numNodes;
		for (int n = 1; n < numNodes; ++n)
		{
			indices.push_back(southPoleIndex);
			indices.push_back(southRingOffset + n-1);
			indices.push_back(southRingOffset + n);
		}
		indices.push_back(southPoleIndex);
		indices.push_back(southRingOffset + numNodes - 1);
		indices.push_back(southRingOffset);

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

				indices.push_back(r*numNodes + n);		// A
				indices.push_back(r*numNodes + n-1);	// B
				indices.push_back((r-1)*numNodes + n);	// C
				indices.push_back(r*numNodes + n-1);	// B
				indices.push_back((r-1)*numNodes + n);	// C
				indices.push_back((r-1)*numNodes + n-1);// D
			}
			//      last    0
			//       |      |
			// r-1 --D------C-- r-1
			//       |      |
			//       |      |
			//   r --B------A-- r
			//       |      |
			//      last    0

			indices.push_back(r*numNodes);					// A
			indices.push_back(r*numNodes + numNodes-1);		// B
			indices.push_back((r-1)*numNodes);				// C
			indices.push_back(r*numNodes + numNodes-1);		// B
			indices.push_back((r-1)*numNodes);				// C
			indices.push_back((r-1)*numNodes + numNodes-1);	// D
		}

		renderer.submit(vertices, indices);
	}
}