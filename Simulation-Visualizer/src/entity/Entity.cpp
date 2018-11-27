#include "Entity.h" 
#include "core/Definitions.h"

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

	glm::vec3 Entity::toLocalFrame(glm::vec3 worldVec) const
	{
		return glm::inverse(glm::toMat3(orientation)) * (worldVec - position);
	}

	glm::vec3 Entity::toWorldFrame(glm::vec3 localVec) const
	{
		//return glm::rotate(orientation, localVec) + position;
		return glm::toMat3(orientation) * localVec + position;
	}

	void Entity::update()
	{
		//velocity += glm::vec3(0.0f, -core::GRAVITY * core::TIME_STEP, 0.0f);
		position += velocity * core::TIME_STEP;
		orientation = glm::normalize(
			glm::angleAxis(
				glm::angle(angVelocity) * core::TIME_STEP, 
				glm::axis(angVelocity)
			) * orientation
		);

	}

	void Entity::applyLinearImpulse(glm::vec3 impulse)
	{
		velocity += impulse/mass;
	}

	void Entity::applyAngularImpulse(glm::vec3 impulse)
	{
		glm::vec3 delta_omega = toWorldFrame(glm::inverse(rotInertia) * toLocalFrame(impulse));
		float angle = static_cast<float>(delta_omega.length());

		angVelocity = glm::angleAxis(angle, delta_omega) * angVelocity;
	}

	void Entity::applyWrenchImpulse(glm::vec3 worldPos, glm::vec3 impulse)
	{
		applyLinearImpulse(impulse);
		applyAngularImpulse(glm::cross(worldPos - position, impulse));
	}

}