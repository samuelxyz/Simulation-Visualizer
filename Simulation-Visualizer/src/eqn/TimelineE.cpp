#include "core/stdafx.h"
#include "TimelineE.h"
#include "EntityE.h"
#include "ContactFloor.h"
#include "ContactEntity.h"

namespace eqn {

	EntityETimestepData* Timestep::getEntityData(const entity::Entity * target)
	{
		for (auto& data : entityData)
			if (data.target == target)
				return &data;
		return nullptr;
	}

	ContactTimestepData* Timestep::getContactData(const entity::Entity * target1, 
		const entity::Entity * target2)
	{
		if (target2 != nullptr)
		{
			for (auto& data : contactData)
				if ((data.target1 == target1 && data.target2 == target2) ||
					(data.target2 == target1 && data.target1 == target2))
					return &data;
		}
		else
		{
			for (auto& data : contactData)
				if (data.target1 == target1)
					return &data;
		}
		return nullptr;
	}

	EntityETimestepData::EntityETimestepData(const EntityE * e)
		: target(e->target),
		q(glm_cast(e->q)), v(glm_cast(e->v)), w(glm_cast(e->w)), 
		qu(glm_cast(e->qu))
	{
	}

	ContactTimestepData::ContactTimestepData(const Contact * c)
		: a1(glm_cast(c->a1)), a2(glm_cast(c->a2))
	{
		const ContactEntity* ce = dynamic_cast<const ContactEntity*>(c);
		if (ce != nullptr)
		{
			target1 = ce->e1->target;
			target2 = ce->e2->target;
			return;
		}
		
		const ContactFloor* cf = dynamic_cast<const ContactFloor*>(c);
		if (cf != nullptr)
		{
			target1 = cf->e->target;
			target2 = nullptr;
			return;
		}
	}
}