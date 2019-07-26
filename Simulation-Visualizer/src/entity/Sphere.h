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
			glm::vec3 position = glm::vec3(0.0f), glm::quat orientation = core::QUAT_IDENTITY,
			glm::vec3 velocity = glm::vec3(0.0f), glm::vec3 angVel = glm::vec3(0.0f),
			float mass = 1.0f, glm::mat3 rotInertia = glm::mat3(1.0f),
			float radius = 1.0f);
		virtual ~Sphere();

		virtual bool containsPoint(glm::vec3 worldPoint) const;
		virtual float getLowestPointZ() const override;
		virtual void render(graphics::Renderer& renderer) const;
		virtual void renderShadow(graphics::Renderer& renderer) const override;

		virtual float getOuterBoundingRadius() const override;
		virtual float getInnerBoundingRadius() const override;
	};

}