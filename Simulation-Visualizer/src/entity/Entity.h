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
			glm::vec3 angVel, float mass, glm::mat3 rotInertia, std::string typeName = "Entity");
		virtual ~Entity();

		virtual core::PathSim* createPathSim() const = 0;

		glm::vec3 toLocalFrame(glm::vec3 worldVec) const;
		glm::vec3 toWorldFrame(glm::vec3 localVec) const;
		virtual bool containsPoint(glm::vec3 worldPoint) const = 0;

		virtual void render(graphics::Renderer& renderer) const = 0;
		virtual void renderShadow(graphics::Renderer& renderer, const glm::vec3& cameraPos) const;
		virtual void renderGUI();
		void renderLabel(const graphics::Camera& camera) const;
		void renderPositionMarker(graphics::Renderer& renderer, const graphics::Camera& camera) const;
		void renderVelocityVector(graphics::Renderer& renderer, const graphics::Camera& camera) const;
		void renderAngularVelocity(graphics::Renderer& renderer, const graphics::Camera& camera) const;
		virtual void update();

		std::string getName() const { return entityName; }
		const glm::vec3& getPosition() const { return position; }
		const glm::quat& getOrientation() const { return orientation; }
		const glm::vec3& getVelocity() const { return velocity; }
		const glm::vec3& getAngVelocity() const { return angVelocity; }
		float getMass() const { return mass; }
		const glm::mat3& getRotInertia() const { return rotInertia; }
		virtual float getShadowRadius() const = 0;
		virtual bool intersectsFloor() const = 0;
		void loadState(core::Timestep&);

		// world frame
		void applyLinearImpulse(glm::vec3);
		void applyAngularImpulse(glm::vec3);
		void applyWrenchImpulse(glm::vec3 worldPos, glm::vec3 impulse);

	public:
		mutable struct GuiBools
		{
			bool gui;
			bool label;
			bool positionMarker;
			bool velocityVector;
			bool angVelocityVector;
		} shouldShow;

	protected:
		std::string typeName;
		std::string entityName;

		glm::vec3 position;
		glm::quat orientation;
		glm::vec3 velocity;
		glm::vec3 angVelocity;
		float mass;
		glm::mat3 rotInertia;
	};
}

