#include "core/stdafx.h"
#include "Cylinder.h"
#include "core/PathSimCylinder.h"

namespace entity {

	Cylinder::Cylinder(std::string entityName,
		glm::vec3 position, glm::quat orientation,
		glm::vec3 velocity, glm::vec3 angVel,
		float mass, glm::mat3 rotInertia, float height, float radius)
		: Entity(entityName, position, orientation, velocity, angVel, mass, rotInertia, "EntityCylinder"),
		height(height), radius(radius),
		visualCylinder(position, orientation, radius, height)
	{
		initializeDragHandles();
	}

	Cylinder::~Cylinder()
	{
	}

	core::PathSim * Cylinder::createPathSim() const
	{
		 return new core::PathSimCylinder;
	}

	bool Cylinder::containsPoint(glm::vec3 worldPoint, bool useCachedOrientation) const
	{
		glm::vec3 localPoint = toLocalFrame(worldPoint, useCachedOrientation);
		return 
			localPoint.z <= height/2.0f &&
			localPoint.z >= -height/2.0f &&
			(localPoint.x * localPoint.x + localPoint.y * localPoint.y) <= radius * radius;
		
	}

	float Cylinder::getLowestPointZ() const
	{
		// Theta is the angle between cylinder axis and world Z-axis

		// should be within [0,1] I think, except for rounding error maybe
		float cosTheta = std::abs((glm::toMat3(orientation) * core::VECTOR_UP).z);
		float sinTheta = std::sqrtf(1.0f - cosTheta * cosTheta);

		// vertical distance between cylinder center and lowest point on tilted cylinder
		float greatestDZ = height * 0.5f * cosTheta + radius * sinTheta;

		return position.z - greatestDZ;
	}

	glm::vec3 Cylinder::guessECP() const
	{
		glm::mat3 orient = glm::toMat3(orientation);
		glm::vec3 localUp = orient * core::VECTOR_UP;
		float cosTheta = std::abs(localUp.z);
		float sinTheta = std::sqrtf(1.0f - cosTheta * cosTheta);

		glm::vec3 hhDown; // halfheight vector downward
		if (std::abs(localUp.z) > 1e-6f)
			hhDown = glm::normalize(localUp/-localUp.z);
		else
			hhDown = glm::vec3(0.0f);

		hhDown *= height/2;

		glm::vec2 rDownPlanar(localUp.x, localUp.y);
		if (glm::length2(rDownPlanar) < 1e-6f)
		{
			rDownPlanar = glm::vec2(0.0f);
		}
		else
		{
			rDownPlanar = glm::normalize(rDownPlanar);
			if (localUp.z < 0.0f)
				rDownPlanar *= -1.0f;
		}
		rDownPlanar *= cosTheta;
		glm::vec3 rDown(rDownPlanar.x, rDownPlanar.y, -sinTheta); // the downward-est radius vector
		rDown *= radius;

		return position + hhDown + rDown;
	}

	void Cylinder::render(graphics::Renderer & renderer) const
	{
		visualCylinder.position = position;
		visualCylinder.orientation = orientation;

		float height = position.z - graphics::FLOOR_Z;
		visualCylinder.shadeFactor = std::max(0.4f - (height/getOuterBoundingRadius() * 0.15f), 0.1f);

		visualCylinder.render(renderer);
	}

	void Cylinder::renderShadow(graphics::Renderer & renderer) const
	{
		//Entity::renderShadow(renderer, 2.8f, position);
		static constexpr float shadowSizeMultiplier = 0.7f;

		// Theta is the angle between cylinder axis and world Z-axis
		// should be within [0,1] I think, except for rounding error maybe
		glm::mat3 orient = glm::toMat3(orientation);
		glm::vec3 localUp = orient * core::VECTOR_UP;
		float cosTheta = std::abs(localUp.z);
		float sinTheta = std::sqrtf(1.0f - cosTheta * cosTheta);
		float groundDrshort = 2 * radius * cosTheta;
		float groundDrlong = 2 * radius;
		float groundDh = height * sinTheta;

		float groundDmax = std::max(std::max(
			groundDh, groundDrshort), groundDrlong);

		float groundDtot = std::sqrt(
			groundDh * groundDh +
			groundDrshort * groundDrshort +
			groundDrlong * groundDrlong
		);
		
		glm::vec3 hhDown; // halfheight vector downward
		if (std::abs(localUp.z) > 1e-6f)
			hhDown = glm::normalize(localUp/-localUp.z);
		else
			hhDown = glm::vec3(0.0f);

		hhDown *= height/2;
		
		glm::vec2 rDownPlanar(localUp.x, localUp.y);
		if (glm::length2(rDownPlanar) < 1e-6f)
			rDownPlanar = glm::vec2(0.0f);
		if (localUp.z < 0.0f)
			rDownPlanar *= -1.0f;
		rDownPlanar *= cosTheta;
		glm::vec3 rDown(rDownPlanar.x, rDownPlanar.y, -sinTheta); // the downward-est radius vector
		rDown *= radius;

		Entity::renderShadow(renderer, shadowSizeMultiplier * groundDtot,
			position + (hhDown*cosTheta + rDown*sinTheta)*0.7f);
	}

	float Cylinder::getOuterBoundingRadius() const
	{
		// makes assumptions, may be overestimate, but it's fast
		return std::max(height, 2.0f * radius) * 0.707f;
		//return 1.4f;
	}

	float Cylinder::getInnerBoundingRadius() const
	{
		return std::min(height/2.0f, radius);
	}


}