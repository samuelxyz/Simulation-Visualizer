#include "core/stdafx.h"
#include "TimelineE.h"
#include "EntityE.h"
#include "ContactFloor.h"
#include "ContactEntity.h"

namespace eqn {

	EntityETimestepData & Timestep::getEntityData(entity::Entity * target)
	{
		for (auto& data : entityData)
			if (data.target == target)
				return data;
	}

	ContactTimestepData & Timestep::getContactData(entity::Entity * target1, 
		entity::Entity * target2)
	{
		if (target2 != nullptr)
		{
			for (auto& data : contactData)
				if ((data.target1 == target1 && data.target2 == target2) ||
					(data.target2 == target1 && data.target1 == target2))
					return data;
		}
		else
		{
			for (auto& data : contactData)
				if (data.target1 == target1)
					return data;
		}

	}

	EntityETimestepData::EntityETimestepData(EntityE * e)
		: target(e->target),
		q(glm_cast(e->q)), v(glm_cast(e->v)), w(glm_cast(e->w)), 
		qu(glm_cast(e->qu))
	{
	}

	ContactTimestepData::ContactTimestepData(Contact * c)
		: a1(glm_cast(c->a1)), a2(glm_cast(c->a2))
	{
		ContactEntity* ce = dynamic_cast<ContactEntity*>(c);
		if (ce != nullptr)
		{
			target1 = ce->e1->target;
			target2 = ce->e2->target;
			return;
		}
		
		ContactFloor* cf = dynamic_cast<ContactFloor*>(c);
		if (cf != nullptr)
		{
			target1 = cf->e->target;
			return;
		}
	}
}