#include "core/stdafx.h"
#include "Contact.h"

namespace eqn
{
	Contact::Contact(double mu, double e_t, double e_o, double e_r)
		: mu(mu), e_t(e_t), e_o(e_o), e_r(e_r)
	{
	}

	void Contact::loadVars(Eigen::VectorXdual & wrt)
	{
		assert(wrt.size() == getN() && 
			"Attempted to load the wrong number of variables into Contact");

		a1 = wrt.head<3>();
		a2 = wrt.segment<3>(3);
		p_t = wrt(6);
		p_o = wrt(7);
		p_r = wrt(8);
		p_n = wrt(9);
		sig = wrt(10);
		l = wrt.tail(getNumContactFuncs());
	}

	Eigen::VectorXdual Contact::getVars()
	{
		Eigen::VectorXdual result(getN());
		result << a1, a2, p_t, p_o, p_r, p_n, sig, l;
		return result;
	}
}
