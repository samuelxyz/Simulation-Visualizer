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
		initializeDragHandles();
	}

	Sphere::~Sphere()
	{
	}

	bool Sphere::containsPoint(glm::vec3 worldPoint) const
	{
		return glm::length(worldPoint - position) <= radius;
	}

	float Sphere::getLowestPointZ() const
	{
		return position.z - radius;
	}

	void Sphere::render(graphics::Renderer & renderer) const
	{
		visualSphere.position = position;
		visualSphere.orientation = orientation;

		visualSphere.render(renderer);
	}

	void Sphere::renderShadow(graphics::Renderer & renderer) const
	{
		Entity::renderShadow(renderer, 1.0f, position);
	}

	float Sphere::getOuterBoundingRadius() const
	{
		return radius;
	}

	float Sphere::getInnerBoundingRadius() const
	{
		return radius;
	}
}