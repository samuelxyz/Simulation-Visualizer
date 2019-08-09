#pragma once
#include "Contact.h"

namespace eqn
{
	struct ContactFloor : public Contact
	{
		ContactFloor(EntityE* e);
		virtual ~ContactFloor() = default;

		EntityE* e;

		virtual void guessVars() override;

		virtual Eigen::Vector3dual a3_Friction() override;
		virtual autodiff::dual c1_Friction() override;
		virtual Eigen::Vector6dual a6_Contact() override;
		virtual Eigen::VectorXdual cX_Contact() override;
		virtual autodiff::dual c1_NonPenetration() override;

		virtual Eigen::Vector6dual getContactImpulses(const EntityE* e) const override;

		virtual std::string getTypeName() const override { return "ContactFloor"; }
		virtual std::string getFullName() const override;
	};
}