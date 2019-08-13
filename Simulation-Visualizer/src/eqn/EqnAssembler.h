#pragma once
#include <vector>
#include "Contact.h"
#include "core/Definitions.h"

namespace entity {
	class Entity;
}

namespace eqn {

	struct EntityE;

	struct TimelineE;

	struct EquationAssembler 
	{
		EquationAssembler();

		// -----------------------------------------------
		// Setup functions, called only to set up system
		// -----------------------------------------------

		// main driver for the below functions
		void setup(bool logOutput = false);
		// anticipates and sets up all combinations of contacts in the system
		void assignContacts();
		// calculates the sizes of z and F (total number of wrt variables)
		int calculateN();
		// sets up the bounds to tell PATH which equations require complementarity
		void setBounds();
		// allocates member arrays z and F on the heap
		void allocateArrays();
		// Loads data from EntityE's/Entities/contacts and sets up the initial guess
		// Order: Individual entities, then individual contacts
		// - Entities: v, w
		// - Contacts: a1, a2, p_t, p_o, p_r, p_n, sig, l
		void initializeZ();

		// -----------------------------------------------
		// Interface functions, used to to operate EqnAssembler
		// -----------------------------------------------

		// adds an entityE
		void addEntityE(EntityE* e);
		// removes an entityE corresponding to the specified entity
		void removeEntity(entity::Entity* e);
		// main driver for step calculation
		int addSteps(eqn::TimelineE& timeline, int currentStep, int numSteps, 
			bool logOutput, bool logCalcTime, bool breakOnConstantMotion = false);
		// shortcut for addSteps()
		int addStepsUntilEnd(eqn::TimelineE& timeline, int currentStep, 
			bool logOutput, bool logCalcTime);
		// record a timestep on the end of the timeline
		void recordTimestep(eqn::TimelineE& timeline, bool recordContacts = true);

		// -----------------------------------------------
		// Called once per step
		// -----------------------------------------------

		// Main step driver function
		// Prepares entities, runs PATH solver, etc
		bool step(eqn::TimelineE& timeline, bool log = true);

		// -----------------------------------------------
		// Called many times per step
		// -----------------------------------------------

		// load changed PATH z variables into entities
		// Order: Individual entities, then individual contacts
		// - Entities: v, w
		// - Contacts: a1, a2, p_t, p_o, p_r, p_n, sig, l
		void loadVars();
		void loadVars(const Eigen::VectorXdual& Z);
		// puts member array z into a dual vector
		Eigen::VectorXdual exportZ();
		
		// Functions called by autodiff

		// Order: Individual entities, then individual contacts
		// - Entities: a6_NewtonEuler
		// - Contacts: a6_Contact, a3_Friction, c1_NonPenetration (p_n), c1_Friction (sig), cX_Contact (l)
		Eigen::VectorXdual calculateFunctions(const Eigen::VectorXdual& Z);

		// attempt to work around arcane template compilation errors in autodiff::forward::jacobian
		Eigen::MatrixXd calculateJacobian(Eigen::VectorXdual& wrt, Eigen::VectorXdual& args, Eigen::VectorXdual& F);

		// functions called by PATH
		int funcEval(int n, double* z, double* F);
		int jacEval(int n, int nnz, double* z, int* column_starting_indices,
			int* len_of_each_column, int* row_index_of_each_value, double* values);
		
		// -----------------------------------------------
		// debug functions
		// -----------------------------------------------
		void printStatus() const;
		void printZ() const;
		void printF() const;
		void printCache() const;

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