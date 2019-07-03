#include "core/stdafx.h"
#include "PathSim.h"
#include "entity/Entity.h"

namespace core {

	PathSim::PathSim()
		: p_x(0.0), p_y(0.0), p_z(0.0), p_xt(0.0), p_yt(0.0), p_zt(0.0),
		mu(0.5), e_o(0.5), e_r(0.5), e_t(0.5)
	{
		// initialize arrays in derived class constructors
		// Do not call any virtual functions (like n()) here.
		// When this constructor is called, the derived part 
		// of the object does not yet exist
	}

	PathSim::~PathSim()
	{
		delete[] z;
		delete[] F;
		delete[] lower_bounds;
		delete[] upper_bounds;
	}

	bool PathSim::addStep(core::Timeline& timeline)
	{
		pathMain(n(), nnz(), &status, z, F, lower_bounds, upper_bounds);

		printZ();

		if (status != 1)
			return false; // solution not found

		// record the resulting timestep
		glm::vec3 velocity(z[0], z[1], z[2]);
		glm::vec3 position(q_xo, q_yo, q_zo);
		position += velocity*core::TIME_STEP;
		//glm::quat orientation(
		//	static_cast<float>(q0_o), 
		//	static_cast<float>(q1_o), 
		//	static_cast<float>(q2_o), 
		//	static_cast<float>(q3_o));
		double& w_x = z[3];
		double& w_y = z[4];
		double& w_z = z[5];
		double q0 = -(2.0*((h*q1_o*w_x)/2.0 - q0_o + (h*q2_o*w_y)/2.0 + (h*q3_o*w_z)/2.0))/std::sqrt(h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4.0);
		double q1 = (2.0*(q1_o + (h*q0_o*w_x)/2.0 + (h*q3_o*w_y)/2.0 - (h*q2_o*w_z)/2.0))/std::sqrt(h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4.0);
		double q2 = (2.0*(q2_o - (h*q3_o*w_x)/2.0 + (h*q0_o*w_y)/2.0 + (h*q1_o*w_z)/2.0))/std::sqrt(h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4.0);
		double q3 = (2.0*(q3_o + (h*q2_o*w_x)/2.0 - (h*q1_o*w_y)/2.0 + (h*q0_o*w_z)/2.0))/std::sqrt(h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4.0);
		glm::quat orientation(
			static_cast<float>(q0),
			static_cast<float>(q1),
			static_cast<float>(q2),
			static_cast<float>(q3));
		glm::vec3 angVelocity(z[3], z[4], z[5]);
		//if (std::isnormal(glm::length2(angVelocity)))
		//	orientation = glm::normalize(
		//		glm::angleAxis(
		//			glm::length(angVelocity) * core::TIME_STEP,
		//			glm::normalize(angVelocity)
		//		) * orientation
		//	);
		glm::vec3 contactPoint(z[6], z[7], z[8]);

		timeline.emplace_back(position, orientation, velocity, angVelocity, contactPoint);

		// update cache variables
		q_xo = position.x;
		q_yo = position.y;
		q_zo = position.z;
		q0_o = orientation.w;
		q1_o = orientation.x;
		q2_o = orientation.y;
		q3_o = orientation.z;
		v_xo = velocity.x;
		v_yo = velocity.y;
		v_zo = velocity.z;
		w_xo = angVelocity.x;
		w_yo = angVelocity.y;
		w_zo = angVelocity.z;

		printCache();

		return true;
	}

	void PathSim::captureTargetState()
	{
		// cache
		q_xo = target->getPosition().x;
		q_yo = target->getPosition().y;
		q_zo = target->getPosition().z;
		q0_o = target->getOrientation().w;
		q1_o = target->getOrientation().x;
		q2_o = target->getOrientation().y;
		q3_o = target->getOrientation().z;
		v_xo = target->getVelocity().x;
		v_yo = target->getVelocity().y;
		v_zo = target->getVelocity().z;
		w_xo = target->getAngVelocity().x;
		w_yo = target->getAngVelocity().y;
		w_zo = target->getAngVelocity().z;
	}

	void PathSim::printCache()
	{
		std::cout << "\nCache:" <<
			"\nq_xo | " << q_xo <<
			"\nq_yo | " << q_yo <<
			"\nq_zo | " << q_zo <<
			"\nq0_o | " << q0_o <<
			"\nq1_o | " << q1_o <<
			"\nq2_o | " << q2_o <<
			"\nq3_o | " << q3_o <<
			"\nv_xo | " << v_xo <<
			"\nv_yo | " << v_yo <<
			"\nv_zo | " << v_zo <<
			"\nw_xo | " << w_xo <<
			"\nw_yo | " << w_yo <<
			"\nw_zo | " << w_zo <<
			"\np_x  | " << p_x  <<
			"\np_y  | " << p_y  <<
			"\np_z  | " << p_z  <<
			"\np_xt | " << p_xt <<
			"\np_yt | " << p_yt <<
			"\np_zt | " << p_zt << std::endl;
	}
}