#include "core/stdafx.h"
#include "ContactEntity.h"
#include "EntityE.h"

namespace eqn
{
	ContactEntity::ContactEntity(EntityE * e1, EntityE * e2, double mu, double e_t, double e_o, double e_r)
		: Contact(mu, e_t, e_o, e_r), e1(e1), e2(e2)
	{ 
	}

	Eigen::Vector3dual ContactEntity::a3_Friction()
	{
		Eigen::Vector6dual nu;
		nu.head<3>() = e1->v;
		nu.tail<3>() = e1->w;

		return Eigen::Vector3dual(
			e_t*e_t * mu * p_n * W_1t.dot(nu) + p_t * sig,
			e_o*e_o * mu * p_n * W_1o.dot(nu) + p_o * sig,
			e_r*e_r * mu * p_n * W_r.dot(nu) + p_r * sig
		);
	}

	// Not sure this is correct
	Eigen::Vector6dual ContactEntity::a6_Contact()
	{
		int m = e1->getNumGeometryFuncs(); // number of constraints in f
		int n = getNumContactFuncs(); // total number of functions/constraints == number of lagrange multipliers

		bool fActive = (active < m);

		Eigen::GeometryGradients gradF_a1 = e1->aX3_GeometryGradients(a1);
		Eigen::GeometryGradients gradG_a2 = e2->aX3_GeometryGradients(a2);

		if (fActive)
		{
			Eigen::Vector3dual sumkLF; // the thing on the left side of eqn 12
			sumkLF.setZero();
			for (int i = 0; i < m; ++i)
			{
				if (i == active)
					sumkLF += gradF_a1.row(i).transpose();
				else
					sumkLF += l(i) * gradF_a1.row(i).transpose();
			}

			Eigen::Vector3dual sumLG;
			sumLG.setZero();
			for (int j = m; j < n; ++j)
			{
				sumLG += l(j) * gradG_a2.row(j).transpose();
			}

			Eigen::Vector6dual result;

			// equation 11
			result.head<3>() = a2-a1 - l(active)*sumkLF;

			// equation 12
			result.tail<3>() = sumkLF + sumLG;

			return result;
		}
		else // gActive
		{
			Eigen::Vector3dual sumLF;
			sumLF.setZero();
			for (int i = 0; i < m; ++i)
			{
				sumLF += l(i) * gradF_a1.row(i).transpose();
			}

			Eigen::Vector3dual sumkLG; // the thing on the left side of eqn 12
			sumkLG.setZero();
			for (int j = m; j < n; ++j)
			{
				if (j == active)
					sumkLG += gradG_a2.row(j).transpose();
				else
					sumkLG += l(j) * gradG_a2.row(j).transpose();
			}

			Eigen::Vector6dual result;

			// equation 11
			result.head<3>() = a2-a1 + l(active)*sumkLG;

			// equation 12
			result.tail<3>() = sumkLG + sumLF;

			return result;
		}
	}

	Eigen::VectorXdual ContactEntity::cX_Contact()
	{
		return Eigen::VectorXdual();
	}

	autodiff::dual ContactEntity::c1_NonPenetration()
	{
		return autodiff::dual();
	}

	void ContactEntity::calculateIntermediates()
	{
		// n is chosen as the thing on either side of equation 12

		r1 = a1 - e1->q;
		r2 = a2 - e2->q;

		int _m = e1->getNumGeometryFuncs(); // number of constraints in f
		int _n = getNumContactFuncs(); // total number of functions/constraints == number of lagrange multipliers

		bool fActive = (active < _m);

		Eigen::GeometryGradients gradF_a1 = e1->aX3_GeometryGradients(a1);
		Eigen::GeometryGradients gradG_a2 = e2->aX3_GeometryGradients(a2);

		if (fActive)
		{
			Eigen::Vector3dual sumkLF; // the thing on the left side of eqn 12
			sumkLF.setZero();
			for (int i = 0; i < _m; ++i)
			{
				if (i == active)
					sumkLF += gradF_a1.row(i).transpose();
				else
					sumkLF += l(i) * gradF_a1.row(i).transpose();
			}
			n = sumkLF;
		}
		else // gActive
		{
			Eigen::Vector3dual sumkLG; // the thing on the left side of eqn 12
			sumkLG.setZero();
			for (int j = _m; j < _n; ++j)
			{
				if (j == active)
					sumkLG += gradG_a2.row(j).transpose();
				else
					sumkLG += l(j) * gradG_a2.row(j).transpose();
			}
			n = -sumkLG;
		}

		n /= autodiff::forward::sqrt(n.dot(n));
		if (std::abs(n(2).val) < 0.99)
			t = Eigen::Vector3dual(0, 0, 1).cross(n);
		else
			t = Eigen::Vector3dual(0, 1, 0).cross(n); // if n is +z then t will be +x
		t /= autodiff::forward::sqrt(t.dot(t));
		o = n.cross(t);

		W_1n.head<3>() = n;
		W_1n.tail<3>() = r1.cross(n);
		W_1t.head<3>() = t;
		W_1t.tail<3>() = r1.cross(t);
		W_1o.head<3>() = o;
		W_1o.tail<3>() = r1.cross(o);

		W_2n.head<3>() = n;
		W_2n.tail<3>() = r2.cross(n);
		W_2t.head<3>() = t;
		W_2t.tail<3>() = r2.cross(t);
		W_2o.head<3>() = o;
		W_2o.tail<3>() = r2.cross(o);

		W_r.head<3>().setZero();
		W_r.tail<3>() = n;

		// currently assumes at least one object involved has only one constraint
		if (e1->getNumGeometryFuncs() == 1)
			active = 0;
		else // if n-m == 1
			active = getNumContactFuncs()-1;
	}

	Eigen::Vector6dual ContactEntity::getContactImpulses(const EntityE * e) const
	{
		if (e == e1)
			return W_1n*p_n + W_1t*p_t + W_1o*p_o + W_r*p_r;
		else
			return -(W_2n*p_n + W_2t*p_t + W_2o*p_o + W_r*p_r);
	}
}