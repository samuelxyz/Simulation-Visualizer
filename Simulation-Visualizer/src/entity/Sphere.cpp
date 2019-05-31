#include "core/stdafx.h"
#include "Sphere.h"

namespace entity {

	Sphere::Sphere(std::string entityName, glm::vec3 position, glm::quat orientation, glm::vec3 velocity, glm::quat angVel,
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
		return (worldPoint - position).length() <= radius;
	}

	void Sphere::render(graphics::Renderer & renderer) const
	{
		visualSphere.position = position;
		visualSphere.orientation = orientation;

		visualSphere.render(renderer);
	}
}