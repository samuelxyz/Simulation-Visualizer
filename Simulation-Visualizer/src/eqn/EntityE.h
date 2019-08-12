#pragma once
#include "glm/glm.hpp"
#include <functional>
#include <vector>
#include "eqn/AutodiffTypes.h"

namespace entity
{
	class Entity;
}

namespace eqn
{
	struct Contact;

	struct EntityE
	{

		const entity::Entity* target;

		// -----------------------------------------------
		// Variables updated multiple times per step
		// -----------------------------------------------

		// "z" variables used by PATH solver
		Eigen::Vector3dual v, w; // position, velocity, angular velocity at end of step

		// intermediate values updated multiple times per step
		Eigen::Vector3dual q;
		Eigen::Vector4dual qu; // orientation quaternion at end of step
		Eigen::Matrix3dual R; // rotation matrix, depends on qu
		Eigen::Matrix3dual Iw; // inertia matrix in world frame, depends on R and Ib

		// -----------------------------------------------
		// "parameter" variables updated once per step
		// -----------------------------------------------

		// "cache" variables, can be updated without external information
		Eigen::Vector3dual q_o, v_o, w_o; // position, velocity, angular velocity previously
		Eigen::Vector4dual qu_o; // orientation quaternion previously

		// other parameter variables
		Eigen::Vector6dual p_app; // externally applied forces and moments

		// -----------------------------------------------
		// "constant" variables, never updated
		// -----------------------------------------------
		autodiff::dual m; // mass
		Eigen::Matrix3dual M, Ib; // mass and inertia matrix in body frame

		// -----------------------------------------------
		// Setup functions
		// -----------------------------------------------

		EntityE(const entity::Entity* target);
		virtual ~EntityE() = default;
		// makes this entity aware of contact impulses from the specified contact
		void addContact(Contact* c);
		// exports member "z" variables as a single vector (used by EqnAssembler setup)
		Eigen::Vector6dual getVars();
		// input target entity variables into member variables
		void captureTargetState();

		// -----------------------------------------------
		// Functions called every step
		// -----------------------------------------------

		// Uses "z" variables to update cache variables at end of step
		// Sets ***_o = ***
		// Make sure "z" variables are up to date; use loadVars() if needed
		void updateCacheVars();

		// -----------------------------------------------
		// Functions called many times per step
		// -----------------------------------------------

		// input changed PATH solver variables into member variables
		void loadVars(Eigen::Vector6dual& nu);
		// uses q_o, qu_o, v, w to calculate q, qu, R, Iw
		void calculateIntermediates();

		// Function evaluations for system of equations

		// "a" prefix means algebraic, "6" means it's a vec6
		Eigen::Vector6dual a6_NewtonEuler();
		// "X" prefix means it's a vecX, unknown dimension
		virtual Eigen::VectorXdual aX_Geometry(Eigen::Vector3dual& v) = 0;
		// Each gradient is a row. The gradient of f_i is row(i)
		virtual Eigen::GeometryGradients aX3_GeometryGradients(Eigen::Vector3dual& v) = 0;

		// -----------------------------------------------
		// num-of-vars functions
		// -----------------------------------------------

		// number of functions f defining the entity's shape
		virtual int getNumGeometryFuncs() const = 0;
		
		std::vector<Contact*> contacts;
	};




}