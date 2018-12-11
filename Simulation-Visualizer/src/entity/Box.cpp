#include "Box.h"

namespace entity {

	Box::Box(std::string entityName, glm::vec3 position, glm::quat orientation, glm::vec3 velocity, glm::quat angVel,
		float mass, glm::mat3 rotInertia,
		float xMin, float xMax, float yMin, float yMax, float zMin, float zMax)
		: Entity(entityName, position, orientation, velocity, angVel, mass, rotInertia, "EntityBox"),
		xMin(xMin), xMax(xMax), yMin(yMin), yMax(yMax), zMin(zMin), zMax(zMax),
		visualBox(position, orientation, xMin, xMax, yMin, yMax, zMin, zMax)
	{
	}

	Box::~Box()
	{
	}

	void Box::render(graphics::Renderer& renderer) const
	{
		visualBox.position = position;
		visualBox.orientation = orientation;

		visualBox.render(renderer);
	}
}