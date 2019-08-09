#pragma once
#include "glm/vec3.hpp"
#include "glm/gtc/quaternion.hpp"
#include <vector>
#include "eqn/EntityE.h"

namespace eqn {

	struct EntityETimestepData
	{
		entity::Entity* target;
		glm::vec3 q, v, w;
		glm::quat qu;
	};

	struct ContactTimestepData
	{
		entity::Entity * target1, * target2;
		glm::vec3 a1, a2;
	};

	struct Timestep
	{
		std::vector<EntityETimestepData> entityData;
		std::vector<ContactTimestepData> contactData;
	};

	struct Timeline : public std::vector<Timestep>
	{
		float timeOf(int timestepIndex, double h) const { return timestepIndex * h; }
		float backTime(double h) const { return timeOf(size() - 1, h); }
	};
}