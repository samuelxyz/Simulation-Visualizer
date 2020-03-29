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

		// -----------------------------------------------
		// "intermediates" - updated many times per step but are not directly solved for
		// -----------------------------------------------
		Eigen::Vector3dual n, t, o, r1, r2; // contact frame/vectors
		Eigen::Vector6dual 
			W_1n, W_1t, W_1o, 
			W_2n, W_2t, W_2o,
			W_r; // contact wrenches

		// -----------------------------------------------
		// Function evaluations for system of equations
		// -----------------------------------------------

		// friction model
		virtual Eigen::Vector3dual a3_Friction() override;
		//virtual autodiff::dual c1_Friction() override;

		// contact constraints
		virtual Eigen::Vector6dual a6_Contact() override;
		virtual Eigen::VectorXdual cX_Contact() override;
		virtual autodiff::dual c1_NonPenetration() override;

		// -----------------------------------------------
		// Preparation functions
		// -----------------------------------------------

		virtual void calculateIntermediates() override;

		// -----------------------------------------------
		// Functions needed for/by entity calculations
		// -----------------------------------------------

		// whether the specified entity corresponds to a1 and the earlier function/lagrange multipliers
		virtual Eigen::Vector6dual getContactImpulses(const EntityE* e) const override;

	};
}