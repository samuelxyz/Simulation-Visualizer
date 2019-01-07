#pragma once

#include "entity/Entity.h"
#include "graphics/content/VisualBox.h"

namespace entity {

	class Box : public Entity
	{
	protected:
		float xMin, xMax, yMin, yMax, zMin, zMax;
		mutable graphics::VisualBox visualBox;

	public:
		Box(std::string entityName, 
			glm::vec3 position = glm::vec3(0.0f), glm::quat orientation = glm::angleAxis(0.0f, glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::vec3 velocity = glm::vec3(0.0f), glm::quat angVel = glm::angleAxis(0.0f, glm::vec3(1.0f, 0.0f, 0.0f)), 
			float mass = 1.0f, glm::mat3 rotInertia = glm::mat3(1.0f),
			float xMin = -1.0f, float xMax = 1.0f, float yMin = -1.0f, float yMax = 1.0f, float zMin = -1.0f, float zMax = 1.0f);
		virtual ~Box();

		virtual bool containsPoint(glm::vec3 worldPoint) const;
		virtual void render(graphics::Renderer& renderer) const;
	};

}