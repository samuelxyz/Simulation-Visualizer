#include "core/stdafx.h"
#include "Cylinder.h"

namespace entity {

	Cylinder::Cylinder(std::string entityName,
		glm::vec3 position, glm::quat orientation,
		glm::vec3 velocity, glm::vec3 angVel,
		float mass, glm::mat3 rotInertia, float height, float radius)
		: Entity(entityName, position, orientation, velocity, angVel, mass, rotInertia, "EntityCylinder"),
		height(height), radius(radius),
		visualCylinder(position, orientation, radius, height)
	{}

	Cylinder::~Cylinder()
	{
	}

	core::PathSim * Cylinder::createPathSim() const
	{
		// TODO
		// return new core::PathSimCylinder;
		return nullptr;
	}

	bool Cylinder::containsPoint(glm::vec3 worldPoint) const
	{
		glm::vec3 localPoint = toLocalFrame(worldPoint);
		return 
			localPoint.z <= height/2.0f &&
			localPoint.z >= -height/2.0f &&
			(localPoint.x * localPoint.x + localPoint.y * localPoint.y) <= radius * radius;
		
	}

	bool Cylinder::intersectsFloor() const
	{
		glm::mat3 orient = glm::toMat3(orientation);

		// Theta is the angle between cylinder axis and world Z-axis

		// should be within [0,1] I think, except for rounding error maybe
		float cosTheta = std::abs((orient * core::VECTOR_UP).z);
		float sinTheta = std::sqrtf(1.0f - cosTheta * cosTheta);

		// vertical distance between cylinder center and lowest point on tilted cylinder
		float greatestDZ = height * 0.5f * cosTheta + radius * sinTheta;

		return position.z - greatestDZ <= core::FLOOR_Z;
	}

	void Cylinder::render(graphics::Renderer & renderer) const
	{
		visualCylinder.position = position;
		visualCylinder.orientation = orientation;

		float height = position.z - core::FLOOR_Z;
		visualCylinder.shadeFactor = std::max(0.4f - (height/getShadowRadius() * 0.15f), 0.1f);

		visualCylinder.render(renderer);
	}

	float Cylinder::getShadowRadius() const
	{
		return std::max(height, 2.0f * radius) * 0.8f;
	}


}