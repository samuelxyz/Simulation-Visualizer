#pragma once

#include "entity/Entity.h"
#include "graphics/content/VisualBox.h"

namespace entity {

	class Box : public Entity
	{
	protected:
		float sizeX, sizeY, sizeZ;
		mutable graphics::VisualBox visualBox;

	public:
		Box(std::string entityName, 
			glm::vec3 position = glm::vec3(0.0f), glm::quat orientation = core::QUAT_IDENTITY,
			glm::vec3 velocity = glm::vec3(0.0f), glm::vec3 angVel = glm::vec3(0.0f), 
			float mass = 1.0f, glm::mat3 rotInertia = glm::mat3(1.0f),
			float sizeX = 1.0f, float sizeY = 1.0f, float sizeZ = 1.0f);
		virtual ~Box();

		//virtual eqn::EntityE* createEntityE() const override;

		virtual bool containsPoint(glm::vec3 worldPoint, bool useCachedOrientation = false) const;
		virtual float getLowestPointZ() const override;
		virtual glm::vec3 guessECP() const override;
		virtual void render(graphics::Renderer& renderer) const;
		virtual void renderShadow(graphics::Renderer& renderer) const override;

		virtual float getOuterBoundingRadius() const override;
		virtual float getInnerBoundingRadius() const override;

		float getSizeX() const { return sizeX; }
		float getSizeY() const { return sizeY; }
		float getSizeZ() const { return sizeZ; }
	};

}