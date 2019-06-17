#pragma once

#include <glm/gtx/quaternion.hpp>

#include "graphics/Renderable.h"
#include "core/Simulation.h"

#include <string>

// Values will be in SI units, radians, etc

namespace entity {

	class Entity : public graphics::Renderable
	{
	public:
		Entity(std::string entityName, glm::vec3 position, glm::quat orientation, glm::vec3 velocity,
			glm::quat angVel, float mass, glm::mat3 rotInertia, std::string typeName = "Entity");
		virtual ~Entity();

		glm::vec3 toLocalFrame(glm::vec3 worldVec) const;
		glm::vec3 toWorldFrame(glm::vec3 localVec) const;
		virtual bool containsPoint(glm::vec3 worldPoint) const = 0;

		virtual void render(graphics::Renderer& renderer) const = 0;
		virtual void renderShadow(graphics::Renderer& renderer) const;
		virtual void renderGUI();
		void renderLabel(const graphics::Camera& camera) const;
		void renderVelocityVector(graphics::Renderer& renderer) const;
		void renderRotationAxis(graphics::Renderer& renderer) const;
		virtual void update(core::Simulation::Parameters&);

		std::string getName() const { return entityName; }
		virtual float getShadowRadius() const = 0;

		// world frame
		void applyLinearImpulse(glm::vec3);
		void applyAngularImpulse(glm::vec3);
		void applyWrenchImpulse(glm::vec3 worldPos, glm::vec3 impulse);

	public:
		mutable struct GuiBools
		{
			bool gui;
			bool label;
			bool velocityVector;
			bool rotationAxis;
		} shouldShow;

	protected:
		std::string typeName;
		std::string entityName;

		glm::vec3 position;
		glm::quat orientation;
		glm::vec3 velocity;
		glm::quat angVelocity;
		float mass;
		glm::mat3 rotInertia;
	};
}

