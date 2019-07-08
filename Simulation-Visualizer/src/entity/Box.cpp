#include "Box.h"
#include "core/Definitions.h"
#include "core/PathSimBox.h"

namespace entity {

	Box::Box(std::string entityName, glm::vec3 position, glm::quat orientation, glm::vec3 velocity, glm::vec3 angVel,
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

	core::PathSim* Box::createPathSim() const
	{
		return new core::PathSimBox;
	}

	bool Box::containsPoint(glm::vec3 worldPoint) const
	{
		glm::vec3 localPoint = toLocalFrame(worldPoint);
		return 
			xMin <= localPoint.x && localPoint.x <= xMax &&
			yMin <= localPoint.y && localPoint.y <= yMax &&
			zMin <= localPoint.z && localPoint.z <= zMax;
	}

	bool Box::intersectsFloor() const
	{
		glm::vec3 vertices[8] {
			{xMin, yMin, zMin},
			{xMin, yMin, zMax},
			{xMin, yMax, zMin},
			{xMin, yMax, zMax},
			{xMax, yMin, zMin},
			{xMax, yMin, zMax},
			{xMax, yMax, zMin},
			{xMax, yMax, zMax}
		};

		for (int i = 0; i < 8; ++i)
			if (toWorldFrame(vertices[i]).z <= core::FLOOR_Z)
				return true;

		return false;
	}

	void Box::render(graphics::Renderer& renderer) const
	{
		visualBox.position = position;
		visualBox.orientation = orientation;

		float height = position.z - core::FLOOR_Z;
		visualBox.shadeFactor = std::max(0.4f - (height/getShadowRadius() * 0.15f), 0.1f);

		visualBox.render(renderer);
	}

	float Box::getShadowRadius() const
	{
		return 2.0f* std::max(std::max(
				std::max(std::abs(xMin), std::abs(xMax)),
				std::max(std::abs(yMin), std::abs(yMax))
			),	std::max(std::abs(zMin), std::abs(zMax)));
	}
}