#pragma once

#include "entity/Entity.h"
#include "graphics/content/VisualCylinder.h"

namespace entity {

	// Default configuration is centered at origin with axis along Z
	class Cylinder :
		public Entity
	{
	protected:
		float height, radius;
		 mutable graphics::VisualCylinder visualCylinder;

	public:
		Cylinder(std::string entityName,
			glm::vec3 position = glm::vec3(0.0f), glm::quat orientation = core::QUAT_IDENTITY,
			glm::vec3 velocity = glm::vec3(0.0f), glm::vec3 angVel = glm::vec3(0.0f),
			float mass = 1.0f, glm::mat3 rotInertia = glm::mat3(1.0f), float height = 1.0f, float radius = 1.0f);
		virtual ~Cylinder();

		virtual core::PathSim* createPathSim() const override;

		virtual bool containsPoint(glm::vec3 worldPoint) const;
		virtual bool intersectsFloor() const override;
		virtual void render(graphics::Renderer& renderer) const;

		virtual float getShadowRadius() const;
	};

}