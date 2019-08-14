#pragma once
#include <vector>
#include <functional>
#include "AutodiffTypes.h"

namespace eqn {

	struct EntityE;

	struct Contact
	{
		Contact(double mu, double e_t, double e_o, double e_r);
		virtual ~Contact() = default;

		// Notes:
		// F order: a6_Contact, a3_Friction, c1_NonPenetration (p_n), c1_Friction (sig), cX_Contact (l)
		// Z order: a1, a2, p_t, p_o, p_r, p_n, sig, l

		// -----------------------------------------------
		// "wrt" variables - updated many times per step
		// -----------------------------------------------
		Eigen::Vector3dual a1, a2; // contact points
		autodiff::dual p_t, p_o, p_r, p_n; // contact impulses
		autodiff::dual sig; // friction sigma (contact sliding velocity)
		Eigen::VectorXdual l; // lagrange multipliers

		// -----------------------------------------------
		// "constant" variables - never updated
		// -----------------------------------------------
		autodiff::dual mu; // friction coefficient
		autodiff::dual e_t, e_o, e_r; // friction ellipsoid

		// -----------------------------------------------
		// Function evaluations for system of equations
		// -----------------------------------------------

		// friction model
		virtual Eigen::Vector3dual a3_Friction() = 0;
		virtual autodiff::dual c1_Friction() = 0;

		// contact constraints
		virtual Eigen::Vector6dual a6_Contact() = 0;
		virtual Eigen::VectorXdual cX_Contact() = 0;
		virtual autodiff::dual c1_NonPenetration() = 0;

		// -----------------------------------------------
		// Preparation functions
		// -----------------------------------------------

		// input changed PATH solver variables into member variables
		void loadVars(Eigen::VectorXdual& wrt);
		// Reverse of loadVars() - exports member "z" variables as a single vector
		Eigen::VectorXdual getVars();
		// assumes this contact's EntityE's are up-to-date
		virtual void guessVars() = 0;

		// -----------------------------------------------
		// Functions needed for/by entity calculations
		// -----------------------------------------------

		// whether the specified entity corresponds to a1 and the earlier function/lagrange multipliers
		virtual bool isFirst(const EntityE* e) const { return true; }
		virtual Eigen::Vector6dual getContactImpulses(const EntityE* e) const = 0;

		// -----------------------------------------------
		// num-of-vars functions
		// -----------------------------------------------

		// the total number of f and g functions == total number of lagrange multipliers
		// == the number of functions in cX_Contact
		unsigned int getNumContactFuncs() const { return l.size(); }
		// the total number of functions/variables
		unsigned int getN() const { return getNumContactFuncs() + 11; }

		// for debug printing and gui and etc
		virtual std::string getTypeName() const = 0;
		virtual std::string getFullName() const = 0;
	};
}