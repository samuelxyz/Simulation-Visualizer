#include "core/stdafx.h"
#include "EqnAssembler.h"
#include "ContactEntity.h"
#include "ContactFloor.h"
#include "EntityE.h"
extern "C" {
#include "path_standalone/Standalone_Path.h"
}
#include "entity/Entity.h"

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

	void EquationAssembler::setup()
	{
		n = calculateN();
		setBounds();
		allocateArrays();
		Eigen::VectorXdual Z = initializeZ();
		for (int i = 0; i < n; ++i)
			z[i] = static_cast<double>(Z(i));

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
			ContactFloor* c = new ContactFloor(entities[i]);
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
			for (int j = 0; j < 9; ++j)
			{
				lower_bounds[i+j] = -core::PATH_INFINITY;
				upper_bounds[i+j] = core::PATH_INFINITY;
			}
			for (int j = 0; j < c->getNumContactFuncs() + 2; ++j)
			{
				lower_bounds[i+j+9] = 0.0;
				upper_bounds[i+j+9] = core::PATH_INFINITY;
			}
			i += c->getN();
		}
	}

	void EquationAssembler::allocateArrays()
	{
		delete[] z;
		delete[] F;
		z = new double[n];
		F = new double[n];
	}

	Eigen::VectorXdual EquationAssembler::initializeZ()
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

		return Z;
	}

	bool EquationAssembler::step(bool log)
	{
		// take care of once-per-step stuff
		for (EntityE* e : entities)
		{
			
		}

		pathMain(n, n*n, &status, z, F, lower_bounds, upper_bounds, log);

		bool success = (status == 1);

		if (success)
		{
			Eigen::VectorXdual Z(n);
			for (int i = 0; i < n; ++i)
				Z(n) = z[n];
			for (EntityE* e : entities)
			{
				e->updateCacheVars();
			}
		}
		
		status = 0;
		return success;
	}

	Eigen::VectorXdual EquationAssembler::calculateFunctions(Eigen::VectorXdual& z)
	{
		// prepare entities and load variables etc
		int i = 0;
		for (EntityE* e : entities)
		{
			Eigen::Vector6dual nu = z.segment<6>(i);
			e->loadVars(nu);
			i += 6;
		}
		for (Contact* c : contacts)
		{
			int num = 11+c->getNumContactFuncs();
			Eigen::VectorXdual v = z.segment(num, i);
			c->loadVars(v);
			i += num;
		}

		// evaluate the functions
		Eigen::VectorXdual F;
		i = 0;

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

	int EquationAssembler::funcEval(int n, double * z, double * F)
	{
		Eigen::VectorXdual zVec(n);
		for (int i = 0; i < n; ++i)
			zVec(i) = z[i];

		Eigen::VectorXdual fVec = calculateFunctions(zVec);
		for (int i = 0; i< n; ++i)
			F[i] = static_cast<double>(fVec(i));

		return 0;
	}

	int EquationAssembler::jacEval(int n, int nnz, double * z, int * column_starting_indices, int * len_of_each_column, int * row_index_of_each_value, double * values)
	{
		Eigen::VectorXdual zVec(n);
		for (int i = 0; i < n; ++i)
			zVec(i) = z[i];

		Eigen::MatrixXdual J_Dense = autodiff::forward::jacobian(calculateFunctions, zVec, zVec);
		Eigen::SparseMatrix<double> J = Eigen::SparseView<Eigen::MatrixXdual>(J_Dense, 1.0).cast<double>();

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

	void EquationAssembler::printZ() const
	{
		std::cout << "Z: \n";
		int i = 0;
		for (EntityE* e : entities)
		{
			std::cout << fmt::sprintf("%s:\n", e->target->getFullName());
			std::cout << fmt::sprintf("z[%i-%i]:\tv: (%f, %f, %f)\n", i, i+2, z[i], z[i+1], z[i+2]);
			std::cout << fmt::sprintf("z[%i-%i]:\tw: (%f, %f, %f)\n", i+3, i+5, z[i+3], z[i+4], z[i+5]);
			i += 6;
		}
		for (Contact* c : contacts)
		{
			std::cout << "Contact:\n";
			std::cout << fmt::sprintf("z[%i-%i]:\ta1: (%f, %f, %f)\n", i, i+2, z[i], z[i+1], z[i+2]);
			std::cout << fmt::sprintf("z[%i-%i]:\ta2: (%f, %f, %f)\n", i+3, i+5, z[i+3], z[i+4], z[i+5]);
			std::cout << fmt::sprintf("z[%i-%i]:\tp_t, p_o, p_r, p_n: (%f, %f, %f, %f)\n", i+6, i+9, z[i+6], z[i+7], z[i+8], z[i+9]);
			std::cout << fmt::sprintf("z[%i]:\tsig: %f", i+10, z[i+10]);

			std::cout << fmt::sprintf("z[%i-%i]:\tl: (", i+11, i+11+c->getNumContactFuncs());
			for (int j = 1; j < c->getNumContactFuncs(); ++j)
				std::cout << ", " << z[i+11+j];
			std::cout << ");\n";

			i += c->getN();
		}
		std::cout << std::endl;
	}

	void EquationAssembler::printF() const
	{
		std::cout << "printF placeholder" << std::endl;
	}
}