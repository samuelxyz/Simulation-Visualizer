#pragma once
#include "glm/vec3.hpp"
#include "glm/gtc/quaternion.hpp"
#include <vector>

namespace entity {
	class Entity;
}

namespace eqn {

	struct EntityE;
	struct Contact;
	struct ContactFloor;
	struct ContactEntity;

	struct EntityETimestepData
	{
		entity::Entity* target;
		glm::vec3 q, v, w;
		glm::quat qu;

		EntityETimestepData(EntityE* e);
	};

	struct ContactTimestepData
	{
		entity::Entity * target1, * target2;
		glm::vec3 a1, a2;

		ContactTimestepData(Contact* c);
	};

	struct Timestep
	{
		std::vector<EntityETimestepData> entityData;
		std::vector<ContactTimestepData> contactData;

		Timestep(): entityData(), contactData() {};

		EntityETimestepData& getEntityData(entity::Entity* target);

		ContactTimestepData& getContactData(entity::Entity* target1, entity::Entity* target2 = nullptr);
	};

	struct TimelineE : public std::vector<Timestep>
	{
		float timeOf(int timestepIndex, double h) const { return static_cast<float>(timestepIndex * h); }
		float backTime(double h) const { return timeOf(size() - 1, h); }
	};
}