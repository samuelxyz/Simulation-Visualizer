#include "core/stdafx.h"
#include "Sphere.h"
#include "core/Definitions.h"

namespace entity {

	Sphere::Sphere(std::string entityName, glm::vec3 position, glm::quat orientation, glm::vec3 velocity, glm::vec3 angVel,
		float mass, glm::mat3 rotInertia, float radius)
		: Entity(entityName, position, orientation, velocity, angVel, mass, rotInertia, "EntitySphere"),
		radius(radius),
		visualSphere(position, orientation, radius)
	{
	}

	Sphere::~Sphere()
	{
	}

	bool Sphere::containsPoint(glm::vec3 worldPoint) const
	{
		return glm::length(worldPoint - position) <= radius;
	}

	bool Sphere::intersectsFloor() const
	{
		return position.z - radius <= graphics::FLOOR_Z;
	}

	void Sphere::render(graphics::Renderer & renderer) const
	{
		visualSphere.position = position;
		visualSphere.orientation = orientation;

		visualSphere.render(renderer);
	}

	float Sphere::getShadowRadius() const
	{
		return 1.0f*radius;
	}

}