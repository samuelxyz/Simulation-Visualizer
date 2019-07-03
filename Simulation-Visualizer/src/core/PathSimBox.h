#pragma once
#include "core/PathSim.h"

namespace core {

	struct PathSimBox :
		public PathSim
	{
		double len, wid, heg;

		PathSimBox();
		virtual ~PathSimBox();

		virtual void setTarget(entity::Entity* target);
		virtual int funcEval(double* z, double* F);
		virtual int jacEval(double* z, int* column_starting_indices,
			int* len_of_each_column, int* row_index_of_each_value, double* values);
		// Returns true if PATH solves successfully
		virtual void captureTargetState();
		virtual void printZ();
		virtual void printConstants();

		virtual int n() const override { return 24; }
	};

}