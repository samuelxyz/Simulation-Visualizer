#pragma once
#include "Contact.h"

namespace eqn 
{
	struct ContactEntity : public Contact
	{
		ContactEntity(EntityE* e1, EntityE* e2, double mu, double e_t, double e_o, double e_r);
		virtual ~ContactEntity() = default;

		EntityE* e1;
		EntityE* e2;

		virtual bool isFirst(const EntityE* e) const override { return e == e1; }
	};
}