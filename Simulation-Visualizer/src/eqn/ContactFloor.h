#pragma once
#include "Contact.h"

namespace eqn
{
	struct ContactFloor : public Contact
	{
		ContactFloor(EntityE* e, double mu, double e_t, double e_o, double e_r);
		virtual ~ContactFloor() = default;

		EntityE* e;

		// -----------------------------------------------
		// "intermediates" - updated many times per step but are not directly solved for
		// -----------------------------------------------
		Eigen::Vector3dual n, t, o, r; // contact frame/vectors
		Eigen::Vector6dual W_n, W_t, W_o, W_r; // contact wrenches

		virtual void guessVars() override;

		virtual Eigen::Vector3dual a3_Friction() override;
		//virtual autodiff::dual c1_Friction() override;
		virtual Eigen::Vector6dual a6_Contact() override;
		virtual Eigen::VectorXdual cX_Contact() override;
		virtual autodiff::dual c1_NonPenetration() override;

		virtual void calculateIntermediates() override;

		virtual Eigen::Vector6dual getContactImpulses(const EntityE* e) const override;

		virtual std::string getTypeName() const override { return "ContactFloor"; }
		virtual std::string getFullName() const override;
	};
}