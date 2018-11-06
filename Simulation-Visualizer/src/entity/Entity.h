#pragma once

#include <glm/gtc/quaternion.hpp>

#include "graphics/Renderable.h"

namespace entity {

	class Entity : public graphics::Renderable
	{
	public:
		Entity(glm::vec3 position, glm::quat orientation, glm::vec3 velocity, glm::quat angVel, float mass, glm::mat3 rotInertia);
		~Entity();

		glm::vec3 toWorldFrame(glm::vec3 localVec) const;

		virtual void render(graphics::Renderer& renderer) const = 0;
		virtual void update();

	private:
		glm::vec3 position;
		glm::quat orientation;
		glm::vec3 velocity;
		glm::quat angVelocity;
		float mass;
		glm::mat3 rotInertia;
	};
}

