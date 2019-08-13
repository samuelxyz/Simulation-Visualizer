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
		const entity::Entity* target;
		glm::vec3 q, v, w;
		glm::quat qu;

		EntityETimestepData(const EntityE* e);
	};

	struct ContactTimestepData
	{
		const entity::Entity * target1, * target2;
		glm::vec3 a1, a2;

		ContactTimestepData(const Contact* c);
	};

	struct Timestep
	{
		std::vector<EntityETimestepData> entityData;
		std::vector<ContactTimestepData> contactData;

		Timestep(): entityData(), contactData() {};

		EntityETimestepData* getEntityData(const entity::Entity* target);

		ContactTimestepData* getContactData(const entity::Entity* target1, const entity::Entity* target2 = nullptr);
	};

	struct TimelineE : public std::vector<Timestep>
	{
		float timeOf(int timestepIndex, double h) const { return static_cast<float>(timestepIndex * h); }
		float backTime(double h) const { return timeOf(size() - 1, h); }
	};
}