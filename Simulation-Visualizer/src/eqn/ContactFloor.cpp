#include "core/stdafx.h"
#include "ContactFloor.h"
#include "EntityE.h"
#include "entity/Entity.h"

namespace eqn
{
	ContactFloor::ContactFloor(EntityE * e, double mu, double e_t, double e_o, double e_r)
		: Contact(mu, e_t, e_o, e_r), e(e)
	{
		l.resize(e->getNumGeometryFuncs() + 1);
	}

	void ContactFloor::guessVars()
	{
		glm::vec3 ecp = e->target->guessECP();
		a1 << ecp.x, ecp.y, ecp.z;
		a2 << ecp.x, ecp.y, 0;

		// TODO maybe better estimates
		p_t = p_o = p_r = p_n = 0;
		sig = 0;
		l.setZero();
	}

	Eigen::Vector3dual ContactFloor::a3_Friction()
	{
		Eigen::Vector6dual nu;
		nu.head<3>() = e->v;
		nu.tail<3>() = e->w;

		return Eigen::Vector3dual(
			e_t*e_t * mu * p_n * W_t.dot(nu) + p_t * sig,
			e_o*e_o * mu * p_n * W_o.dot(nu) + p_o * sig,
			e_r*e_r * mu * p_n * W_r.dot(nu) + p_r * sig
		);
	}

	Eigen::Vector6dual ContactFloor::a6_Contact()
	{
		// this version specialized for floor contact
		// the last lagrange multiplier matches with the floor
		// and is the active constraint

		int m = e->getNumGeometryFuncs(); // number of geometry functions for entity e1
		int n = getNumContactFuncs(); // number of functions == number of lagrange multipliers

		Eigen::GeometryGradients gradF_a1 = e->aX3_GeometryGradients(a1);
		Eigen::Vector3dual gradG_a2(0, 0, 1);

		Eigen::Vector3dual sumLF;
		sumLF.setZero();
		for (int i = 0; i < m; ++i)
		{
			sumLF += l(i) * gradF_a1.row(i).transpose();
		}

		Eigen::Vector6dual result;

		// equation 11
		result.head<3>() = a2-a1 + l(active)*gradG_a2; 

		// equation 12
		result.tail<3>() = sumLF + gradG_a2;

		return result;
	}

	Eigen::VectorXdual ContactFloor::cX_Contact()
	{
		int m = e->getNumGeometryFuncs(); // number of geometry functions for entity e1
		int n = getNumContactFuncs(); // number of functions == number of lagrange multipliers
		int active = n-1; // active constraint is the floor

		Eigen::VectorXdual f_a1 = e->aX_Geometry(a1);
		autodiff::dual& g_a2 = a2(2);

		Eigen::VectorXdual result(n);
		result.head(m) = -f_a1;
		result(n-1) = -g_a2;

		return result;
	}

	autodiff::dual ContactFloor::c1_NonPenetration()
	{
		return a1(2);
	}

	void ContactFloor::calculateIntermediates()
	{
		n << 0, 0, 1;
		t << 1, 0, 0;
		o << 0, 1, 0;
		r = a1 - this->e->q;

		W_n.head<3>() = n;
		W_n.tail<3>() = r.cross(n);
		W_t.head<3>() = t;
		W_t.tail<3>() = r.cross(t);
		W_o.head<3>() = o;
		W_o.tail<3>() = r.cross(o);
		W_r.head<3>().setZero();
		W_r.tail<3>() = n;

		active = getNumContactFuncs()-1; // active constraint is the floor
	}

	Eigen::Vector6dual ContactFloor::getContactImpulses(const EntityE * e) const
	{
		return W_n*p_n + W_t*p_t + W_o*p_o + W_r*p_r;
	}

	std::string ContactFloor::getFullName() const
	{
		return getTypeName() + "(" + e->target->getFullName() + ")";
	}

}