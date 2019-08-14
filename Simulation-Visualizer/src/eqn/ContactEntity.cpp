#include "core/stdafx.h"
#include "ContactEntity.h"
#include "EntityE.h"

namespace eqn
{
	ContactEntity::ContactEntity(EntityE * e1, EntityE * e2, double mu, double e_t, double e_o, double e_r)
		: Contact(mu, e_t, e_o, e_r), e1(e1), e2(e2)
	{
		// bind contact functions to entities

	//	int i = 0;
	//	for (auto& f : e1->contactAlgFuncs)
	//	{
	//		contactAlgFuncs[i] = [&]() { return f(a1_x, a1_y, a1_z); };
	//		++i;
	//	}
	//	for (auto& f : e2->contactAlgFuncs)
	//	{
	//		contactAlgFuncs[i] = [&]() { return f(a1_x, a1_y, a1_z); };
	//		++i;
	//	}

	//	i = 0;
	//	for (auto& f : e2->contactComplFuncs)
	//	{
	//		contactComplFuncs[i] = [&]() { return f(a2_x, a2_y, a2_z); };
	//		++i;
	//	}
	//	for (auto& f : e2->contactComplFuncs)
	//	{
	//		contactComplFuncs[i] = [&]() { return f(a2_x, a2_y, a2_z); };
	//	}
	}
}