#pragma once
#include "core/PathSim.h"

namespace core {

	struct PathSimCylinder :
		public PathSim
	{
		double radius, heg;

		PathSimCylinder();
		virtual ~PathSimCylinder();

		virtual void setTarget(entity::Entity* target) override;
		virtual int funcEval(double* z, double* F) override;
		virtual int jacEval(double* z, int* column_starting_indices,
			int* len_of_each_column, int* row_index_of_each_value, double* values) override;
		// Returns true if PATH solves successfully
		virtual void captureTargetState(bool updateGuesses = true) override;
		virtual void printZ() const override;
		virtual void printConstants() const override;

		virtual int n() const override { return 21;	}
	};

}