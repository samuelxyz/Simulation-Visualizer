#include "core/stdafx.h"
#include "EqnAssembler.h"
#include "ContactEntity.h"
#include "ContactFloor.h"
extern "C" {
#include "path_standalone/Standalone_Path.h"
}
#include "entity/Entity.h"
#include "TimelineE.h"
#include "EntityE.h"

namespace eqn {

	const std::array<std::string, 11> EquationAssembler::statusCodes = {
		"Not yet started",
		"Solution found",
		"No progress",
		"Major iteration limit",
		"Cumulative iteration limit",
		"Time limit",
		"User interrupt",
		"Bound error",
		"Domain error",
		"Infeasible",
		"Internal error"
	};

	EquationAssembler::EquationAssembler()
		: entities(), contacts(), structureNeedsRefresh(true), h(core::TIME_STEP),
		mu(0.5), e_t(0.5), e_o(0.5), e_r(0.5)
	{
	}

	void EquationAssembler::setup(bool logOutput)
	{
		assignContacts();
		n = calculateN();
		setBounds();
		allocateArrays();
		initializeZ();

		if (logOutput)
		{
			std::cout << fmt::sprintf("EntityE list (total %i entities):\n", entities.size());
			for (EntityE* e : entities)
				std::cout << "\t" + e->target->getFullName() + "\n";
			std::cout << fmt::sprintf("Contact list (total %i contacts):\n", contacts.size());
			for (Contact* c : contacts)
				std::cout << "\t" + c->getFullName() + "\n";
			std::cout << fmt::sprintf("System of equations contains %i variables.\n", n);
		}

		structureNeedsRefresh = false;
	}

	void EquationAssembler::assignContacts()
	{
		contacts.clear();
		//contacts.reserve(some amount)

		//// assign contacts between entities
		//for (unsigned int i = 0; i < entities.size()-1; ++i)
		//{
		//	for (unsigned int j = i+1; j < entities.size(); ++j)
		//	{
		//		contacts.push_back(new ContactEntity(entities[i], entities[j]));
		//		entities[i]->addContact(contacts.back());
		//		entities[j]->addContact(contacts.back());
		//	}
		//}

		// assign floor contacts
		for (unsigned int i = 0; i < entities.size(); ++i)
		{
			ContactFloor* c = new ContactFloor(entities[i], mu, e_t, e_o, e_r);
			contacts.push_back(c);
			entities[i]->addContact(c);
		}
	}

	int EquationAssembler::calculateN()
	{
		// - Entities: a6_NewtonEuler (v, w)
		// - Contacts: a6_Contact (a1, a2), a3_Friction (p_t, p_o, p_r), 
		//             c1_NonPenetration (p_n), c1_Friction (sig)
		int n = entities.size() * 6 + contacts.size() * 11;
		

		for (Contact* c : contacts)
		{
			n += c->getNumContactFuncs(); // cX_Contact
		}

		return n;
	}

	void EquationAssembler::setBounds()
	{
		delete[] lower_bounds;
		delete[] upper_bounds;
		lower_bounds = new double[n];
		upper_bounds = new double[n];

		// - Entities: a6_NewtonEuler (v, w)
		// - Contacts: a6_Contact (a1, a2), a3_Friction (p_t, p_o, p_r), 
		//             c1_NonPenetration (p_n), c1_Friction (sig), cX_Contact (l)
		unsigned int i = 0;
		for (; i < 6*entities.size(); ++i)
		{
			lower_bounds[i] = -core::PATH_INFINITY;
			upper_bounds[i] = core::PATH_INFINITY;
		}
		for (Contact* c : contacts)
		{
			unsigned int j = 0;
			for (; j < 9; ++j)
			{
				lower_bounds[i+j] = -core::PATH_INFINITY;
				upper_bounds[i+j] = core::PATH_INFINITY;
			}
			for (; j < c->getN(); ++j)
			{
				lower_bounds[i+j] = 0.0;
				upper_bounds[i+j] = core::PATH_INFINITY;
			}
			i += j;
		}
		assert(i == n && "Something goofed while setting bounds for PATH");
	}

	void EquationAssembler::allocateArrays()
	{
		delete[] z;
		delete[] F;
		z = new double[n];
		F = new double[n];
	}

	void EquationAssembler::addEntityE(EntityE * e)
	{
		entities.push_back(e);
		structureNeedsRefresh = true;
	}

	void EquationAssembler::removeEntity(entity::Entity * e)
	{
		auto it = std::find_if(entities.begin(), entities.end(), 
			[e](EntityE* ee){ return ee->target == e; } );
		assert(it != entities.end() && 
			"EquationAssembler::removeEntity() tried to remove an EntityE that wasn't already there");
		entities.erase(it);

		structureNeedsRefresh = true;
	}

	int EquationAssembler::addSteps(TimelineE& timeline, int currentStep, int numSteps, 
		bool logOutput, bool logCalcTime, bool breakOnConstantMotion)
	{
		if (logOutput)
			if (breakOnConstantMotion)
				std::cout << fmt::sprintf("Beginning addSteps() to fulfill auto-calculate request on timestep %i\n", currentStep);
			else
				std::cout << fmt::sprintf("Beginning addSteps() to fulfill request of %i steps on timestep %i\n", numSteps, currentStep);

		bool zInitialized = false;
		if (structureNeedsRefresh)
		{
			if (logOutput)
				std::cout << "Structure needs refresh, calling setup()\n";
			setup(logOutput);
			zInitialized = true;
			structureNeedsRefresh = false;
		}

		START_TIMING;

		// start from current state, replacing any existing version of events
		timeline.erase(timeline.begin() + currentStep, timeline.end());
		if (!zInitialized)
			initializeZ();
		recordTimestep(timeline, false);
		if (logOutput)
			std::cout << "Captured current situation to timeline. Beginning PATH steps\n";

		int successes = 0;
		int stepsWithAllConstantMotion = 0;
		
		successes = [&]() // if this lambda returns negative, indicates failure
		{ 
			while (successes < numSteps)
			{
				// try a path step
				//pathSim->target->loadState(timeline.back());
				//pathSim->captureTargetState(!pathStreaking); // if we're in a streak then no need to update guesses

				if (logOutput)
				{
					printZ();
					printCache();
				}

				if (step(timeline, logOutput))
				{
					++successes;
					if (logOutput)
						if (breakOnConstantMotion)
							std::cout << "Successful step " << successes << "\n";
						else
						std::cout << "Successful step " << successes << "/" << numSteps << "\n";

				}
				else
				{
					// path not successful
					if (logOutput)
						std::cout << "Step failed, trying again with new guess\n";
					 // try again with fresh guess
					initializeZ();
					if (logOutput)
					{
						printZ();
						printCache();

						Eigen::VectorXdual Z = exportZ();
						Eigen::VectorXdual F;
						std::cout << "Initial Jacobian:\n" << calculateJacobian(Z, Z, F) << "\n";
						for (int i = 0; i < n; ++i)
							this->F[i] = F(i).val;
						printF();
					}

					if (step(timeline, logOutput))
					{
						// success with new guess
						if (logOutput)
							std::cout << "Succeeded with new guess. Continuing\n";
						++successes;

						continue;
					}
					else
					{
						if (logOutput)
							if (breakOnConstantMotion)
								std::cout << "Step still failed, stopping calculations after " << successes << " steps\n";
							else
							std::cout << "Step still failed, stopping calculations after " << successes << "/" << numSteps << " requested steps\n";
						return -successes; // Return negative to indicate failure
					}
				}

				static constexpr int constantMotionThreshold = 300; // arbitrary but probably pretty good for the purpose
				if (breakOnConstantMotion && successes > 1)
				{
					int last = timeline.size()-1;
					
					bool motionConstant = true;
					// for each entity
					for (unsigned int i = 0; i < timeline.back().entityData.size(); ++i)
					{
						if (timeline[last].entityData[i].v == timeline[last-1].entityData[i].v &&
							timeline[last].entityData[i].w == timeline[last-1].entityData[i].w)
						{
							continue;
						}
						else
						{
							motionConstant = false;
							break;
						}
					}

					if (motionConstant)
						++stepsWithAllConstantMotion;
					else
						stepsWithAllConstantMotion = 0;

					if (stepsWithAllConstantMotion > constantMotionThreshold)
					{
						if (logOutput)
							std::cout << "Motion seems to have become constant after " << successes << " steps, stopping calculations\n";
						return successes; // everything stopped, we're done. Return positive
					}
				}
			}

			// reached the end
			if (logOutput)
				std::cout << "Finished " << successes << "/" << numSteps << " requested steps, stopping calculations\n";
			return successes;
		}(); // end lambda

		if (logCalcTime)
			std::cout << "Calculation time: " << STOP_TIMING_AND_GET_MICROSECONDS / 1e6f;
		if (logCalcTime || logOutput)
			std::cout << std::endl;
		return successes;
	}

	int EquationAssembler::addStepsUntilEnd(TimelineE& timeline, int currentStep, 
		bool logOutput, bool logCalcTime)
	{
		static constexpr int max = 1000;

		return addSteps(timeline, currentStep, max, logOutput, logCalcTime, true);
	}

	void EquationAssembler::recordTimestep(eqn::TimelineE & timeline, bool recordContacts)
	{
		timeline.emplace_back();

		Timestep& timestep = timeline.back();

		for (EntityE* e : entities)
			timestep.entityData.emplace_back(e);

		if (recordContacts)
			for (Contact* c : contacts)
				timestep.contactData.emplace_back(c);
	}

	void EquationAssembler::initializeZ()
	{
		Eigen::VectorXdual Z(calculateN());
		int i = 0;

		for (EntityE* e : entities)
		{
			e->captureTargetState();
			Z.segment<6>(i) = e->getVars();
			i += 6;
		}

		for (Contact* c : contacts)
		{
			c->guessVars();
			Z.segment(i, c->getN()) = c->getVars();
			i += c->getN();
		}

		for (int i = 0; i < n; ++i)
			z[i] = static_cast<double>(Z(i));
	}

	bool EquationAssembler::step(eqn::TimelineE& timeline, bool log)
	{
		pathMain(n, n*n, &status, z, F, lower_bounds, upper_bounds, log);

		bool success = (status == 1);

		if (log)
		{
			printStatus();
			printZ();
			printF();
		}

		if (success)
		{
			loadVars();
			for (EntityE* e : entities)
			{
				e->updateCacheVars();
			}

			recordTimestep(timeline, true);
		}
		
		status = 0;
		return success;
	}

	void EquationAssembler::loadVars()
	{
		loadVars(exportZ());
	}

	void EquationAssembler::loadVars(const Eigen::VectorXdual& Z)
	{
		int i = 0;
		for (EntityE* e : entities)
		{
			Eigen::Vector6dual nu = Z.segment<6>(i);
			e->loadVars(nu);
			i += 6;
		}
		for (Contact* c : contacts)
		{
			int num = c->getN();
			Eigen::VectorXdual v = Z.segment(i, num);
			c->loadVars(v);
			i += num;
		}
	}

	Eigen::VectorXdual EquationAssembler::exportZ()
	{
		return exportZ(z);
	}

	Eigen::VectorXdual EquationAssembler::exportZ(double * z)
	{
		Eigen::VectorXdual Z(n);
		for (int i = 0; i < n; ++i)
			Z(i) = z[i];
		return Z;
	}

	Eigen::VectorXdual EquationAssembler::calculateFunctions(const Eigen::VectorXdual& Z)
	{
		// prepare entities and load variables etc
		loadVars(Z);

		// evaluate the functions
		Eigen::VectorXdual F(n);
		int i = 0;

		for (EntityE* e : entities)
		{
			F.segment<6>(i) = e->a6_NewtonEuler();
			i += 6;
		}

		// Notes:
		// F order: a6_Contact, a3_Friction, c1_NonPenetration (p_n), c1_Friction (sig), cX_Contact (l)
		// Z order: a1, a2, p_t, p_o, p_r, p_n, sig, l
		for (Contact* c : contacts)
		{
			// algebraic
			F.segment<6>(i) = c->a6_Contact();
			i += 6;
			F.segment<3>(i) = c->a3_Friction();
			i += 3;

			// complementarity
			F(i) = c->c1_NonPenetration();
			++i;
			F(i) = c->c1_Friction();
			++i;
			F.segment(i, c->getNumContactFuncs()) = c->cX_Contact();
			i += c->getNumContactFuncs();
		}

		return F;
	}

	Eigen::MatrixXd EquationAssembler::calculateJacobian(Eigen::VectorXdual & wrt, Eigen::VectorXdual & args, Eigen::VectorXdual & F)
	{
		const int n = wrt.size();

		wrt[0].grad = 1.0;
		F = calculateFunctions(args);
		wrt[0].grad = 0.0;

		const int m = F.size();

		Eigen::MatrixXd J(m, n);

		for (int i = 0; i < m; ++i)
			J(i, 0) = F[i].grad;

		for (int j = 1; j < n; ++j)
		{
			wrt[j].grad = 1.0;
			F = calculateFunctions(args);
			wrt[j].grad = 0.0;

			for (int i = 0; i < m; ++i)
				J(i, j) = F[i].grad;
		}

		return J;
	}

	int EquationAssembler::funcEval(int n, double * z, double * F)
	{
		Eigen::VectorXdual Z = exportZ(z);
		Eigen::VectorXdual fVec = calculateFunctions(Z);
		for (int i = 0; i< n; ++i)
			F[i] = static_cast<double>(fVec(i));

		return 0;
	}

	int EquationAssembler::jacEval(int n, int nnz, double * z, int * column_starting_indices, int * len_of_each_column, int * row_index_of_each_value, double * values)
	{
		Eigen::VectorXdual Z = exportZ(z);

		Eigen::VectorXdual F;
		//Eigen::MatrixXd J_Dense = autodiff::forward::jacobian(
		//	&EquationAssembler::calculateFunctions, autodiff::wrt(Z), autodiff::forward::at(Z), F);
		Eigen::MatrixXd J_Dense = calculateJacobian(Z, Z, F);
		Eigen::SparseMatrix<double> J = Eigen::SparseView<Eigen::MatrixXd>(J_Dense, 1.0);

		J.makeCompressed();
		int val_index = 0;
		int* outer = J.outerIndexPtr();

		for (int col = 0; col < J.cols(); ++col)
		{	// the next two lines are equivalent
			//column_starting_indices[col] = outer[col]+1;
			column_starting_indices[col] = val_index+1; // these indices need to be from 1

			len_of_each_column[col] = outer[col+1] - outer[col];

			for (Eigen::SparseMatrix<double>::InnerIterator it(J, col); it; ++it)
			{
				values[val_index] = it.value();
				row_index_of_each_value[val_index] = it.row()+1; // indices need to be from 1
				++val_index;
			}
		}

		return 0;
	}

	void EquationAssembler::printStatus() const
	{
		std::cout << fmt::sprintf("Status: %i (%s)\n", status, statusCodes[status]);
	}

	void EquationAssembler::printZ() const
	{
		std::cout << "Z: \n";
		unsigned int i = 0;
		for (EntityE* e : entities)
		{
			std::cout << fmt::sprintf("%s:\n", e->target->getFullName());
			std::cout << fmt::sprintf("z[%2i-%2i]:\tv: (%f, %f, %f)\n", i, i+2, z[i], z[i+1], z[i+2]);
			std::cout << fmt::sprintf("z[%2i-%2i]:\tw: (%f, %f, %f)\n", i+3, i+5, z[i+3], z[i+4], z[i+5]);
			i += 6;
		}
		for (Contact* c : contacts)
		{
			std::cout << fmt::sprintf("%s:\n", c->getFullName());
			std::cout << fmt::sprintf("z[%2i-%2i]:\ta1: (%f, %f, %f)\n", i, i+2, z[i], z[i+1], z[i+2]);
			std::cout << fmt::sprintf("z[%2i-%2i]:\ta2: (%f, %f, %f)\n", i+3, i+5, z[i+3], z[i+4], z[i+5]);
			std::cout << fmt::sprintf("z[%2i-%2i]:\tp_t, p_o, p_r, p_n: (%f, %f, %f, %f)\n", i+6, i+9, z[i+6], z[i+7], z[i+8], z[i+9]);
			std::cout << fmt::sprintf("z[%5i]:\tsig: %f\n", i+10, z[i+10]);

			if (c->getNumContactFuncs() == 1)
				std::cout << fmt::sprintf("z[%5i]:\tl: (%f", i+11, z[i+11]);
			else
				std::cout << fmt::sprintf("z[%2i-%2i]:\tl: (%f", i+11, i+11+c->getNumContactFuncs(), z[i+11]);
			for (unsigned int j = 1; j < c->getNumContactFuncs(); ++j)
				std::cout << ", " << z[i+11+j];
			std::cout << ")\n";

			i += c->getN();
		}
		std::cout << std::endl;
	}

	void EquationAssembler::printF() const
	{
		std::cout << "F:\n";
		unsigned int i = 0;
		for (EntityE* e : entities)
		{
			std::cout << fmt::sprintf("%s:\n", e->target->getFullName());
			std::cout << fmt::sprintf("F[%2i-%2i]:\ta6_NewtonEuler: (%f, %f, %f, %f, %f, %f)\n",
				i, i+5, F[i], F[i+1], F[i+2], F[i+3], F[i+4], F[i+5]);
			i += 6;
		}
		for (Contact* c : contacts)
		{
			std::cout << fmt::sprintf("%s:\n", c->getFullName());
			std::cout << fmt::sprintf("F[%2i-%2i]:\ta6_Contact: (%f, %f, %f, %f, %f, %f)\n",
				i, i+5, F[i], F[i+1], F[i+2], F[i+3], F[i+4], F[i+5]);
			std::cout << fmt::sprintf("F[%2i-%2i]:\ta3_Friction: (%f, %f, %f)\n",
				i+6, i+8, F[i+6], F[i+7], F[i+8]);
			std::cout << fmt::sprintf("F[%5i]:\tc1_NonPenetration: %f\n",
				i+9, F[i+9]);
			std::cout << fmt::sprintf("F[%5i]:\tc1_Friction: %f\n",
				i+10, F[i+10]);

			if (c->getNumContactFuncs() == 1)
				std::cout << fmt::sprintf("F[%5i]:\tcX_Contact: (%f", i+11, F[i+11]);
			else
				std::cout << fmt::sprintf("F[%2i-%2i]:\tcX_Contact: (%f", i+11, i+11+c->getNumContactFuncs(), F[i+11]);
			for (unsigned int j = 1; j < c->getNumContactFuncs(); ++j)
				std::cout << ", " << F[i+11+j];
			std::cout << ")\n";

			i += c->getN();
		}
		std::cout << std::endl;
	}

	void EquationAssembler::printCache() const
	{
		std::cout << "Cache:" << std::endl;
		for (EntityE* e : entities)
		{
			std::cout << fmt::sprintf("%s:\n", e->target->getFullName());
			std::cout << fmt::sprintf("q_o:\t(%f, %f, %f)\n", e->q_o(0), e->q_o(1), e->q_o(2));
			std::cout << fmt::sprintf("qu_o:\t(%f, %f, %f, %f)\n", e->qu_o(0), e->qu_o(1), e->qu_o(2), e->qu_o(3));
			std::cout << fmt::sprintf("v_o:\t(%f, %f, %f)\n", e->v_o(0), e->v_o(1), e->v_o(2));
			std::cout << fmt::sprintf("w_o:\t(%f, %f, %f)\n", e->w_o(0), e->w_o(1), e->w_o(2));
		}
		std::cout << std::endl;
	}
}