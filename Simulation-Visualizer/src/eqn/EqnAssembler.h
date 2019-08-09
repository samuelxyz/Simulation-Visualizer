#pragma once
#include <vector>
#include "Contact.h"

namespace eqn {

	struct EntityE;

	struct EquationAssembler 
	{
		// -----------------------------------------------
		// Setup functions, called only to set up system
		// -----------------------------------------------

		// main driver for the below functions
		void setup();
		// anticipates and sets up all combinations of contacts in the system
		void assignContacts();
		// calculates the sizes of z and F (total number of wrt variables)
		int calculateN();
		// sets up the bounds to tell PATH which equations require complementarity
		void setBounds();
		// allocates member arrays z and F on the heap
		void allocateArrays();

		// -----------------------------------------------
		// Called once per step
		// -----------------------------------------------

		// Loads data from EntityE's/contacts and sets up the initial guess
		// Order: Individual entities, then individual contacts
		// - Entities: v, w
		// - Contacts: a1, a2, p_t, p_o, p_r, p_n, sig, l
		Eigen::VectorXdual initializeZ();

		// Main step driver function
		// Prepares entities, runs PATH solver, etc
		bool step(bool log = true);

		// -----------------------------------------------
		// Called many times per step
		// -----------------------------------------------
		
		// Functions called by autodiff

		// Order: Individual entities, then individual contacts
		// - Entities: a6_NewtonEuler
		// - Contacts: a6_Contact, a3_Friction, c1_NonPenetration (p_n), c1_Friction (sig), cX_Contact (l)
		Eigen::VectorXdual calculateFunctions(Eigen::VectorXdual& z);

		// functions called by PATH
		int funcEval(int n, double* z, double* F);
		int jacEval(int n, int nnz, double* z, int* column_starting_indices,
			int* len_of_each_column, int* row_index_of_each_value, double* values);
		
		// -----------------------------------------------
		// debug functions
		// -----------------------------------------------
		virtual void printZ() const;
		virtual void printF() const;

		// -----------------------------------------------
		// Member variables
		// -----------------------------------------------

		std::vector<EntityE*> entities;
		std::vector<Contact*> contacts;

		bool structureNeedsRefresh;
		double h;

		int n;
		int status;
		double* z;
		double* F;
		double* lower_bounds;
		double* upper_bounds;

		static const std::array<std::string, 11> statusCodes;
	};
}