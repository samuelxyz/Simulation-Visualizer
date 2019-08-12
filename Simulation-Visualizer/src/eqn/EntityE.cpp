// This file is named "Entity2.cpp" so as not to conflict with "entity/Entity.cpp"

#include "core/stdafx.h"
#include "core/Definitions.h"
#include "EntityE.h"
#include "Contact.h"
#include "entity/Entity.h"

namespace eqn {

	EntityE::EntityE(const entity::Entity * target)
		: target(target), 
		m(target->getMass()), M(m * Eigen::Matrix3dual::Identity()), Ib()
	{
		auto rot = target->getRotInertia();
		Ib.setZero();
		Ib(0, 0) = rot[0][0];
		Ib(1, 1) = rot[1][1];
		Ib(2, 2) = rot[2][2];

		captureTargetState();
	}

	void EntityE::captureTargetState()
	{
		v_o = dual_cast(target->getVelocity());
		w_o = dual_cast(target->getAngVelocity());
		q_o = dual_cast(target->getPosition());
		qu_o = dual_cast(target->getOrientation());

		// guesses
		v = v_o;
		w = w_o;

		calculateIntermediates();
	}

	void EntityE::addContact(Contact * c)
	{
		contacts.push_back(c);
	}

	void EntityE::loadVars(Eigen::Vector6dual & nu)
	{
		v = nu.head<3>();
		w = nu.tail<3>();
		
		calculateIntermediates();
	}

	Eigen::Vector6dual EntityE::getVars()
	{
		Eigen::Vector6dual result;
		result << v, w;
		return result;
	}

	void EntityE::calculateIntermediates()
	{
		q = q_o + v*h;
		qu <<
			-(2.0*((h*qu_o(1)*w(0))/2.0 - qu_o(0) + (h*qu_o(2)*w(1))/2.0 + (h*qu_o(3)*w(2))/2.0))/std::sqrt(h*h*w(0)*w(0) + h*h*w(1)*w(1) + h*h*w(2)*w(2) + 4.0),
			(2.0*(qu_o(1) + (h*qu_o(0)*w(0))/2.0 + (h*qu_o(3)*w(1))/2.0 - (h*qu_o(2)*w(2))/2.0))/std::sqrt(h*h*w(0)*w(0) + h*h*w(1)*w(1) + h*h*w(2)*w(2) + 4.0),
			(2.0*(qu_o(2) - (h*qu_o(3)*w(0))/2.0 + (h*qu_o(0)*w(1))/2.0 + (h*qu_o(1)*w(2))/2.0))/std::sqrt(h*h*w(0)*w(0) + h*h*w(1)*w(1) + h*h*w(2)*w(2) + 4.0),
			(2.0*(qu_o(3) + (h*qu_o(2)*w(0))/2.0 - (h*qu_o(1)*w(1))/2.0 + (h*qu_o(0)*w(2))/2.0))/std::sqrt(h*h*w(0)*w(0) + h*h*w(1)*w(1) + h*h*w(2)*w(2) + 4.0);
		R <<
			qu(0)*qu(0) + qu(1)*qu(1) - qu(2)*qu(2) - qu(3)*qu(3), 2*qu(1)*qu(2) - 2*qu(0)*qu(3), 2*qu(0)*qu(2) + 2*qu(1)*qu(3),
			2*qu(0)*qu(3) + 2*qu(1)*qu(2), qu(0)*qu(0) - qu(1)*qu(1) + qu(2)*qu(2) - qu(3)*qu(3), 2*qu(2)*qu(3) - 2*qu(0)*qu(1),
			2*qu(1)*qu(3) - 2*qu(0)*qu(2), 2*qu(0)*qu(1) + 2*qu(2)*qu(3), qu(0)*qu(0) - qu(1)*qu(1) - qu(2)*qu(2) + qu(3)*qu(3);
		Iw = R * Ib * R.transpose();
	}

	void EntityE::updateCacheVars()
	{
		v_o = v;
		w_o = w;
		q_o = q;
		qu_o = qu;
	}

	Eigen::Vector6dual EntityE::a6_NewtonEuler()
	{
		Eigen::Vector6dual LHS;
		LHS.head<3>() = M * (v-v_o);
		LHS(3) += g*h*m;
		LHS.tail<3>() = Iw*(w-w_o) + h * w.cross(Iw * w);

		Eigen::Vector6dual RHS = p_app;
		for (Contact* c : contacts)
			RHS += c->getContactImpulses(this);

		return RHS - LHS;
	}
}