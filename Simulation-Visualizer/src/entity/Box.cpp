#include "Box.h"
#include "core/Definitions.h"
#include "core/PathSimBox.h"

namespace entity {

	Box::Box(std::string entityName, glm::vec3 position, glm::quat orientation, glm::vec3 velocity, glm::vec3 angVel,
		float mass, glm::mat3 rotInertia,
		float sizeX, float sizeY, float sizeZ)
		: Entity(entityName, position, orientation, velocity, angVel, mass, rotInertia, "EntityBox"),
		sizeX(sizeX), sizeY(sizeY), sizeZ(sizeZ), 
		visualBox(position, orientation, -sizeX/2, sizeX/2, -sizeY/2, sizeY/2, -sizeZ/2, sizeZ/2)
	{
		initializeDragHandles();
	}

	Box::~Box()
	{
	}

	core::PathSim* Box::createPathSim() const
	{
		return new core::PathSimBox;
	}

	bool Box::containsPoint(glm::vec3 worldPoint, bool useCachedOrientation) const
	{
		glm::vec3 localPoint = toLocalFrame(worldPoint, useCachedOrientation);
		return
			std::abs(localPoint.x) <= sizeX/2 &&
			std::abs(localPoint.y) <= sizeY/2 &&
			std::abs(localPoint.z) <= sizeZ/2;
	}

	float Box::getLowestPointZ() const
	{
		float
			xMax = sizeX/2, xMin = -xMax,
			yMax = sizeY/2, yMin = -yMax,
			zMax = sizeZ/2, zMin = -zMax;

		glm::vec3 vertices[8] {
			{xMin, yMin, zMin},
			{xMin, yMin, zMax},
			{xMin, yMax, zMin},
			{xMin, yMax, zMax},
			{xMax, yMin, zMin},
			{xMax, yMin, zMax},
			{xMax, yMax, zMin},
			{xMax, yMax, zMax}
		};

		float lowestZ = position.z;
		for (int i = 0; i < 8; ++i)
		{
			float thisZ = toWorldFrame(vertices[i], i != 0).z;
			if (thisZ < lowestZ)
				lowestZ = thisZ;
		}

		return lowestZ;
	}

	glm::vec3 Box::guessECP() const
	{
		float
			xMax = sizeX/2, xMin = -xMax,
			yMax = sizeY/2, yMin = -yMax,
			zMax = sizeZ/2, zMin = -zMax;

		glm::vec3 vertices[]
		{
			toWorldFrame(glm::vec3{ xMin, yMin, zMin }), // 0
			toWorldFrame(glm::vec3{ xMin, yMin, zMax }, true), // 1
			toWorldFrame(glm::vec3{ xMin, yMax, zMin }, true), // 2
			toWorldFrame(glm::vec3{ xMin, yMax, zMax }, true), // 3
			toWorldFrame(glm::vec3{ xMax, yMin, zMin }, true), // 4
			toWorldFrame(glm::vec3{ xMax, yMin, zMax }, true), // 5
			toWorldFrame(glm::vec3{ xMax, yMax, zMin }, true), // 6
			toWorldFrame(glm::vec3{ xMax, yMax, zMax }, true), // 7
		};

		int lowestCount = 0;
		glm::vec3* lowest[4];
		float lowestZ = position.z;
		for (glm::vec3& v : vertices)
		{
			if (v.z < lowestZ)
				lowestZ = v.z;
		}
		for (glm::vec3& v : vertices)
		{
			if (std::abs(v.z - lowestZ) < 1e-3f)
				lowest[lowestCount++] = &v;
		}

		glm::vec3 averagePos(0.0f);
		for (int i = 0; i < lowestCount; ++i)
			averagePos += *(lowest[i]);
		averagePos /= static_cast<float>(lowestCount);

		return averagePos;
	}

	void Box::render(graphics::Renderer& renderer) const
	{
		visualBox.position = position;
		visualBox.orientation = orientation;

		float height = position.z - graphics::FLOOR_Z;
		visualBox.shadeFactor = std::max(0.4f - (height/getOuterBoundingRadius() * 0.15f), 0.1f);

		visualBox.render(renderer);
	}

	void Box::renderShadow(graphics::Renderer & renderer) const
	{
		// idea for new method: average of vertex positions, weighted by proximity to ground

		static constexpr float shadowSizeMultiplier = 0.7f;
		
		// Theta is the angle between cube vertical axis and world Z-axis
		// should be within [0,1] I think, except for rounding error maybe
		glm::mat3 orient = glm::toMat3(orientation);
		float cosTheta = std::abs((orient * core::VECTOR_UP).z);
		float sinTheta = std::sqrtf(1.0f - cosTheta * cosTheta);
		float groundDx = sizeX * cosTheta;
		float groundDy = sizeY * cosTheta;
		float groundDz = sizeZ * sinTheta;

		//float groundDmax = std::max(std::max(
		//	groundDx, groundDy), groundDz);

		float groundDtot = std::sqrt(
			groundDx * groundDx +
			groundDy * groundDy +
			groundDz * groundDz
		);

		float
			xMax = sizeX/2, xMin = -xMax,
			yMax = sizeY/2, yMin = -yMax,
			zMax = sizeZ/2, zMin = -zMax;

		glm::vec3 vertices[]
		{
			toWorldFrame(glm::vec3{ xMin, yMin, zMin }), // 0
			toWorldFrame(glm::vec3{ xMin, yMin, zMax }, true), // 1
			toWorldFrame(glm::vec3{ xMin, yMax, zMin }, true), // 2
			toWorldFrame(glm::vec3{ xMin, yMax, zMax }, true), // 3
			toWorldFrame(glm::vec3{ xMax, yMin, zMin }, true), // 4
			toWorldFrame(glm::vec3{ xMax, yMin, zMax }, true), // 5
			toWorldFrame(glm::vec3{ xMax, yMax, zMin }, true), // 6
			toWorldFrame(glm::vec3{ xMax, yMax, zMax }, true), // 7
		};

		glm::vec3 average(0.0f);
		float totalWeights = 0.0f;
		float rad = getOuterBoundingRadius();
		float lowestZ = position.z - rad;
		float highestZ = position.z + rad;
		for (glm::vec3& v : vertices)
		{
			float weight = core::mapRange(v.z, lowestZ, highestZ, 1, 0.5f);
			average += weight*v;
			totalWeights += weight;
		}
		average /= totalWeights;

		Entity::renderShadow(renderer, shadowSizeMultiplier * groundDtot,
			average);


		//glm::vec3 average(0.0f);
		//float totalWeights = 0.0f;
		//float lowestZ = position.z;
		//for (glm::vec3& v : vertices)
		//{
		//	if (v.z < lowestZ)
		//		lowestZ = v.z;
		//}
		//float highestZ = position.z + (position.z - lowestZ); // symmetric
		//float range = highestZ - lowestZ;
		//for (glm::vec3& v : vertices)
		//{
		//	float heightRel = v.z - lowestZ;
		//	float weight = core::mapRange(heightRel, 0, range, 1, 0.5f);
		//	average += weight*v;
		//	totalWeights += weight;
		//}
		//average /= totalWeights;
		//
		//Entity::renderShadow(renderer, shadowSizeMultiplier * groundDtot,
		//	average);

		// -------------------------------------------------------------------

		//// halfwidth vectors
		//glm::vec3 hwx = orient * glm::vec3(1.0f, 0.0f, 0.0f);
		//glm::vec3 hwy = orient * glm::vec3(0.0f, 1.0f, 0.0f);
		//glm::vec3 hwz = orient * glm::vec3(0.0f, 0.0f, 1.0f);

		//// get them pointing down
		//hwx /= (std::abs(hwx.z) < 1e-6f) ? 1.0f : -(hwx.z/std::abs(hwx.z));
		//hwy /= (std::abs(hwy.z) < 1e-6f) ? 1.0f : -(hwy.z/std::abs(hwy.z));
		//hwz /= (std::abs(hwz.z) < 1e-6f) ? 1.0f : -(hwz.z/std::abs(hwz.z));

		//auto compareZ = [](const glm::vec3& a, const glm::vec3& b) -> bool { return (a.z < b.z); };
		//// whichever has the most negative z is the closest to vertical
		//const glm::vec3* hwVert = &(std::min(std::min(
		//	hwx, hwy, compareZ), hwz, compareZ));


		//glm::vec3* hwFace[2]; // the vectors that aren't hwVert
		//float sizeFace[2];
		//int i = 0;
		//if (&hwx != hwVert)
		//{
		//	hwFace[i] = &hwx;
		//	sizeFace[i] = sizeX;
		//	++i;
		//}
		//if (&hwy != hwVert)
		//{
		//	hwFace[i] = &hwy;
		//	sizeFace[i] = sizeY;
		//	++i;
		//}
		//if (&hwz != hwVert)
		//{
		//	hwFace[i] = &hwz;
		//	sizeFace[i] = sizeZ;
		//	++i;
		//}

		
		// z component guaranteed <= 0.707
		// z = sin(pitch) of each hw vector
		// make the bigger pitch one full size
		// make the smaller pitch one smaller

		//float biggerZAbs = std::max(std::abs(hwFace[0]->z), std::abs(hwFace[1]->z));
		//glm::vec3 towardLowestPoint;
		////if (biggerZAbs < 1e-6f)
		////	towardLowestPoint =
		////	(*hwFace[0])*(sizeFace[0]/2) +
		////	(*hwFace[1])*(sizeFace[1]/2);
		////else
		//	towardLowestPoint =
		//	(*hwFace[0])*(-(hwFace[0]->z)/*/biggerZAbs*/)*(sizeFace[0]/2) +
		//	(*hwFace[1])*(-(hwFace[1]->z)/*/biggerZAbs*/)*(sizeFace[1]/2);

		//glm::vec2 tlpGround(towardLowestPoint.x, towardLowestPoint.y);

		//Entity::renderShadow(renderer, shadowSizeMultiplier * groundDmax ,
		//	position + (towardLowestPoint*sinTheta + (*hwVert)*cosTheta)*0.7f);

		// -----------------------------------------------------------------------


		//glm::vec3 longestGroundHalfwidth(0.0f);
		//if (groundDx >= groundDy && groundDx >= groundDz)
		//	longestGroundHalfwidth = orient * glm::vec3(1.0f, 0.0f, 0.0f) * (sizeX/2);
		//else if (groundDy >= groundDz)
		//	longestGroundHalfwidth = orient * glm::vec3(0.0f, 1.0f, 0.0f) * (sizeY/2);
		//else
		//	longestGroundHalfwidth = orient * core::VECTOR_UP * (sizeZ/2);

		//// get the one pointing downwards
		//if (longestGroundHalfwidth.z > 0)
		//	longestGroundHalfwidth *= -1.0f;

		//Entity::renderShadow(renderer, shadowSizeMultiplier * groundDtot, 
		//	position + longestGroundHalfwidth*sinTheta);

		//glm::vec3 localUp = orient * core::VECTOR_UP/* * (height/2)*/; // in world frame. this could be resized to a halfheight vector but unnecessary
		//glm::vec3 towardLowestPoint;
		//if (localUp.z > 1e-6f)
		//	towardLowestPoint = glm::normalize(localUp/localUp.z);
		//else
		//	towardLowestPoint = glm::vec3(0.0f);

		//towardLowestPoint *= groundDmax * cosTheta;

		//Entity::renderShadow(renderer, shadowSizeMultiplier * groundDtot,
		//	position + towardLowestPoint*sinTheta*0.7f);
	}

	float Box::getOuterBoundingRadius() const
	{
		// this makes some assumptions, might be overestimate, 
		// but it's pretty fast

		return 0.866f * std::max(std::max(
				sizeX, sizeY), sizeZ);

		//return 1.732f;
	}

	float Box::getInnerBoundingRadius() const
	{
		return std::min(std::min(
			sizeX, sizeY), sizeZ
		) / 2.0f;
	}
}