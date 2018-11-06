#include "Entity.h"

namespace entity {

	Entity::Entity(glm::vec3 position, glm::quat orientation,
		glm::vec3 velocity, glm::quat angVel,
		float mass, glm::mat3 rotInertia)
		: position(position), orientation(orientation),
		velocity(velocity), angVelocity(angVel),
		mass(mass), rotInertia(rotInertia)
	{
	}

	Entity::~Entity()
	{
	}

	glm::vec3 Entity::toWorldFrame(glm::vec3 localVec) const
	{
		glm::mat3 R = glm::mat3_cast(orientation);
		return R * localVec + position;
	}

	void Entity::update()
	{
		position += velocity;
		orientation = glm::normalize(angVelocity * orientation);

	}

}