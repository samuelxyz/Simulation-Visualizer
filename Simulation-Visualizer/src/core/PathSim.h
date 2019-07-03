#pragma once

#include "core/Definitions.h"
#include "core/Simulation.h"

namespace entity {
	class Entity;
}

namespace core
{ 

	struct PathSim
	{
		entity::Entity* target;

		static constexpr double g = static_cast<double>(core::GRAVITY);
		static constexpr double h = static_cast<double>(core::TIME_STEP);
		static constexpr double PATH_INFINITY = 1e20;

		// cache
		double q_xo, q_yo, q_zo;
		double q0_o, q1_o, q2_o, q3_o;
		double v_xo, v_yo, v_zo;
		double w_xo, w_yo, w_zo;
		// applied impulses
		double p_x, p_y, p_z, p_xt, p_yt, p_zt;

		// constants
		double m;
		double mu;
		double I_xx, I_yy, I_zz;
		double e_o, e_r, e_t;

		// path vars (dynamic arrays)
		//static constexpr int n = 24; // number of unknowns
		//static constexpr int nnz = 24*24; // overestimate of jacobian nonzeros
		int status;
		double* z;
		double* F;
		double* lower_bounds;
		double* upper_bounds;

		PathSim();
		virtual ~PathSim();
		// number of unknowns. This would be static except it also has to be virtual
		virtual int n() const = 0;
		// overestimate of jacobian nonzeros. This would be static except it also has to be virtual
		int nnz() const { return n() * n(); }
		virtual void setTarget(entity::Entity*) = 0;
		virtual int funcEval(double* z, double* F) = 0;
		virtual int jacEval(double* z, int* column_starting_indices,
			int* len_of_each_column, int* row_index_of_each_value, double* values) = 0;
		// Uses PATH variables, capture from target entity first if necessary
		// Adds step to end of timeline and returns true if successful
		// Does not update target entity
		virtual bool addStep(core::Timeline& timeline);
		// Updates cache only. z should be updated in subclass overrides
		virtual void captureTargetState();
		virtual void printZ() = 0;
		void printCache();
		virtual void printConstants() = 0;
	};

}