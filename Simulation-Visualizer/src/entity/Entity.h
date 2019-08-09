#pragma once

#include <glm/gtx/quaternion.hpp>

#include "graphics/Renderable.h"
#include "core/Simulation.h"
#include "io/MouseDragTarget.h"
#include "io/DragHandle.h"
#include "io/DragHandleTarget.h"

#include <string>
#include <array>

// Values will be in SI units, radians, etc

namespace entity {

	class Entity : public graphics::Renderable, public io::MouseDragTarget, public io::DragHandleTarget
	{
	public:
		Entity(std::string entityName, glm::vec3 position, glm::quat orientation, glm::vec3 velocity,
			glm::vec3 angVel, float mass, glm::mat3 rotInertia, std::string typeName = "Entity");
		virtual ~Entity();

		virtual core::PathSim* createPathSim() const = 0;

		glm::vec3 toLocalFrame(glm::vec3 worldVec, bool useCachedOrientation = false) const;
		glm::vec3 toWorldFrame(glm::vec3 localVec, bool useCachedOrientation = false) const;
		virtual bool containsPoint(glm::vec3 worldPoint, bool useCachedOrientation = false) const = 0;

		virtual void render(graphics::Renderer& renderer) const = 0;

		// refPoint determines the size/darkness of the shadow. To get default behavior, use this->position
		void renderShadow(graphics::Renderer& renderer, float shadowSizeMultiplier, const glm::vec3& refPoint) const;
		void renderShadow(graphics::Renderer&, const glm::vec2& centerPos, float radius, float centerA) const;
		virtual void renderShadow(graphics::Renderer& renderer) const = 0;
		virtual void renderGUI();
		void renderLabel(const graphics::Camera& camera) const;
		void renderPositionMarker(graphics::Renderer& renderer, const graphics::Camera& camera) const;
		void renderVelocityVector(graphics::Renderer& renderer, const graphics::Camera& camera) const;
		void renderAngularVelocity(graphics::Renderer& renderer, const graphics::Camera& camera) const;
		virtual void update();

		std::string getName() const { return entityName; }
		std::string getTypeName() const { return typeName; }
		std::string getFullName() const { return getTypeName() + ": " + getName(); }
		const glm::vec3& getPosition() const { return position; }
		const glm::quat& getOrientation() const { return orientation; }
		const glm::vec3& getVelocity() const { return velocity; }
		const glm::vec3& getAngVelocity() const { return angVelocity; }
		float getMass() const { return mass; }
		const glm::mat3& getRotInertia() const { return rotInertia; }

		// >= the maximum extent of the entity
		virtual float getOuterBoundingRadius() const = 0;

		// radius of the largest sphere that will fit inside the entity
		virtual float getInnerBoundingRadius() const = 0;

		virtual glm::vec3 guessECP() const = 0;
		virtual float getLowestPointZ() const = 0;
		bool intersectsFloor() const;

		void loadState(core::Timestep&);
		virtual void applyDragHandleResult(const glm::vec3&) override;
		virtual glm::vec3 getDragHandlePosition() const override;
		virtual bool isDragHandleVisible() const override;
		std::array<io::DragHandle, 4>& getDragHandles() { return dragHandles; }
		void displace(const glm::vec3&);

		// world frame
		void applyLinearImpulse(glm::vec3);
		void applyAngularImpulse(glm::vec3);
		void applyWrenchImpulse(glm::vec3 worldPos, glm::vec3 impulse);

		virtual bool handleRightMouseDrag(graphics::Camera& camera, const glm::vec2& dx) override;

	public:
		mutable struct VisibilitySettings
		{
			bool body;
			bool gui;
			bool label;
			bool positionMarker;
			bool velocityVector;
			bool angVelocityVector;
			bool shadow;
			bool dragHandles;
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

		struct GuiVars
		{
			float applyRotateAngle;
			glm::vec3 applyRotateAxis;
			glm::vec3 applyLinImpulse;
			glm::vec3 applyAngImpulse;
		} guiVars;

		std::array<io::DragHandle, 4> dragHandles;

		// Call this in subclass constructors
		void initializeDragHandles();
	};
}

