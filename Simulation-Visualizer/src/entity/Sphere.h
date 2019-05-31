#pragma once
#include "Entity.h"
#include "graphics/content/VisualSphere.h"

namespace entity {

	class Sphere :
		public Entity
	{
	protected:
		float radius;
		mutable graphics::VisualSphere visualSphere;
		
	public:
		Sphere(std::string entityName,
			glm::vec3 position = glm::vec3(0.0f), glm::quat orientation = glm::angleAxis(0.0f, glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::vec3 velocity = glm::vec3(0.0f), glm::quat angVel = glm::angleAxis(0.0f, glm::vec3(1.0f, 0.0f, 0.0f)),
			float mass = 1.0f, glm::mat3 rotInertia = glm::mat3(1.0f),
			float radius = 1.0f);
		virtual ~Sphere();

		virtual bool containsPoint(glm::vec3 worldPoint) const;
		virtual void render(graphics::Renderer& renderer) const;
	};

}