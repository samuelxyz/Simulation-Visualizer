#pragma once

#include "entity/Entity.h"

namespace entity {

	class Box : public Entity
	{
	public:
		Box();
		Box(glm::vec3 position, glm::quat orientation);
		Box(glm::vec3 position, glm::quat orientation, glm::vec3 velocity, glm::quat angVel);
		Box(glm::vec3 position, glm::quat orientation, glm::vec3 velocity, glm::quat angVel, float mass, glm::mat3 rotInertia);
		Box(glm::vec3 position, glm::quat orientation, glm::vec3 velocity, glm::quat angVel, float mass, glm::mat3 rotInertia,
			float xMin, float xMax, float yMin, float yMax, float zMin, float zMax);
		~Box();

		virtual void render(graphics::Renderer& renderer) const;

	private:
		float xMin, xMax, yMin, yMax, zMin, zMax;
	};

}