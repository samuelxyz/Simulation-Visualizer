#include "core/stdafx.h"
#include "Simulation.h"

#include "core/Definitions.h"
#include "entity/Box.h"
#include "graphics/Camera.h"
#include <imgui/imgui.h>
#include "graphics/content/VisualSphere.h"
extern "C" {
#include "path_standalone/Standalone_Path.h"
}

namespace core {

	Simulation::Simulation()
		: pathSim(), entities(), environment(), parameters(), timeline()
	{
	}

	Simulation::Environment::Environment()
		: floor()
	{
		// floor
		glm::vec4 colorGray(0.8f, 0.8f, 0.8f, 1.0f);
		floor.push_back(graphics::ColoredVertex { graphics::COLOR_WHITE, glm::vec3( 0.0f,  0.0f, -50.0f) });
		floor.push_back(graphics::ColoredVertex { colorGray , glm::vec3(-5.0f, -5.0f, -50.0f) });
		floor.push_back(graphics::ColoredVertex { colorGray , glm::vec3(-5.0f,  5.0f, -50.0f) });
		floor.push_back(graphics::ColoredVertex { colorGray , glm::vec3( 5.0f,  5.0f, -50.0f) });
		floor.push_back(graphics::ColoredVertex { colorGray , glm::vec3( 5.0f, -5.0f, -50.0f) });
	}

	Simulation::Parameters::Parameters()
		: timePaused(true), playbackTime(0.0f), currentStep(0),
		showShadows(true), showEnvironment(true), showContactPoint(true),
		playbackSpeed(1.0), loopPlayback(false)
	{
	}

	Simulation::PathSim::PathSim()
		: p_x(0.0), p_y(0.0), p_z(0.0), p_xt(0.0), p_yt(0.0), p_zt(0.0),
		mu(0.5), e_o(0.5), e_r(0.5), e_t(0.5)
	{
		for (int i = 0; i < n; ++i)
		{
			if (i <= 14)
				lower_bounds[i] = -PATH_INFINITY;
			else
				lower_bounds[i] = 0.0;
			upper_bounds[i] = PATH_INFINITY;
			z[i] = 0.0;
		}
	}

	void Simulation::PathSim::setBox(entity::Box* box)		
	{
		this->box = box;
		m = box->getMass();
		I_xx = box->getRotInertia()[0][0];
		I_yy = box->getRotInertia()[1][1];
		I_zz = box->getRotInertia()[2][2];
		len = box->getSizeX();
		wid = box->getSizeY();
		heg = box->getSizeZ();
		captureBoxState();
		printConstants();
	}

	int Simulation::PathSim::cubeFuncEval(double* z, double* F)
	{
		double& v_x = z[0];
		double& v_y = z[1];
		double& v_z = z[2];
		double& w_x = z[3];
		double& w_y = z[4];
		double& w_z = z[5];

		double& a1_x = z[6];
		double& a1_y = z[7];
		double& a1_z = z[8];

		double& a2_x = z[9];
		double& a2_y = z[10];
		double& a2_z = z[11];

		double& p_t = z[12];
		double& p_o = z[13];
		double& p_r = z[14];

		double& sig = z[15];

		double& l1 = z[16];
		double& l2 = z[17];
		double& l3 = z[18];
		double& l4 = z[19];
		double& l5 = z[20];
		double& l6 = z[21];
		double& l7 = z[22];

		double& p_n = z[23];

		double q0 = -(2.0*((h*q1_o*w_x)/2.0 - q0_o + (h*q2_o*w_y)/2.0 + (h*q3_o*w_z)/2.0))/std::sqrt(h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4.0);
		double q1 = (2.0*(q1_o + (h*q0_o*w_x)/2.0 + (h*q3_o*w_y)/2.0 - (h*q2_o*w_z)/2.0))/std::sqrt(h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4.0);
		double q2 = (2.0*(q2_o - (h*q3_o*w_x)/2.0 + (h*q0_o*w_y)/2.0 + (h*q1_o*w_z)/2.0))/std::sqrt(h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4.0);
		double q3 = (2.0*(q3_o + (h*q2_o*w_x)/2.0 - (h*q1_o*w_y)/2.0 + (h*q0_o*w_z)/2.0))/std::sqrt(h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4.0);

		double q_x = q_xo+h*v_x;
		double q_y = q_yo+h*v_y;
		double q_z = q_zo+h*v_z;

		double R11 = q0*q0 + q1*q1 - q2*q2 - q3*q3;
		double R12 = 2*q1*q2 - 2*q0*q3;
		double R13 = 2*q0*q2 + 2*q1*q3;
		double R21 = 2*q0*q3 + 2*q1*q2;
		double R22 = q0*q0 - q1*q1 + q2*q2 - q3*q3;
		double R23 = 2*q2*q3 - 2*q0*q1;
		double R31 = 2*q1*q3 - 2*q0*q2;
		double R32 = 2*q0*q1 + 2*q2*q3;
		double R33 = q0*q0 - q1*q1 - q2*q2 + q3*q3;

		double I11 = I_xx*R11*R11 + I_yy*R12*R12 + I_zz*R13*R13;
		double I12 = I_xx*R21*R11 + I_yy* R22*R12 + I_zz*R23*R13;
		double I13 = I_xx*R31*R11 + I_yy* R32*R12 + I_zz*R33*R13;
		double I21 = I12;
		double I22 = I_xx*R21*R21 + I_yy*R22*R22 + I_zz*R23*R23;
		double I23 = I_xx*R31*R21 + I_yy*R32*R22 + I_zz*R33*R23;
		double I31 = I13;
		double I32 = I23;
		double I33 = I_xx*R31*R31 + I_yy*R32*R32 + I_zz*R33*R33;

		F[0] = p_t + p_x - m*(v_x - v_xo);
		F[1] = p_o + p_y - m*(v_y - v_yo);
		F[2] = p_n + p_z - m*(v_z - v_zo) - g*h*m;

		F[3] = p_xt		  - I11*(w_x - w_xo) - I12*(w_y - w_yo) - I13*(w_z - w_zo) + p_n*(a1_y - q_y) - p_o*(a1_z - q_z) - h*w_yo*(I31*w_xo + I32*w_yo + I33*w_zo) + h*w_zo*(I21*w_xo + I22*w_yo + I23*w_zo);
		F[4] = p_yt		  - I21*(w_x - w_xo) - I22*(w_y - w_yo) - I23*(w_z - w_zo) - p_n*(a1_x - q_x) + p_t*(a1_z - q_z) + h*w_xo*(I31*w_xo + I32*w_yo + I33*w_zo) - h*w_zo*(I11*w_xo + I12*w_yo + I13*w_zo);
		F[5] = p_r + p_zt - I31*(w_x - w_xo) - I32*(w_y - w_yo) - I33*(w_z - w_zo) + p_o*(a1_x - q_x) - p_t*(a1_y - q_y) - h*w_xo*(I21*w_xo + I22*w_yo + I23*w_zo) + h*w_yo*(I11*w_xo + I12*w_yo + I13*w_zo);

		//// Friction Model without complementarity equation

		F[6] = p_t*sig + e_t*e_t*mu*p_n*v_x - e_t*e_t*mu*p_n*w_z*(a1_y - q_y) + e_t*e_t*mu*p_n*w_y*(a1_z - q_z);
		F[7] = p_o*sig + e_o*e_o*mu*p_n*v_y + e_o*e_o*mu*p_n*w_z*(a1_x - q_x) - e_o*e_o*mu*p_n*w_x*(a1_z - q_z);
		F[8] = mu*p_n*w_z*e_r*e_r + p_r*sig;

		//// contact constraints without complementarity equation

		F[9] = a2_x - a1_x;
		F[10] = a2_y - a1_y;
		F[11] = a2_z - a1_z + l7;

		F[12] = R11*l2 - R11*l1 - R12*l3 + R12*l4 - R13*l5 + R13*l6;
		F[13] = R21*l2 - R21*l1 - R22*l3 + R22*l4 - R23*l5 + R23*l6;
		F[14] = R31*l2 - R31*l1 - R32*l3 + R32*l4 - R33*l5 + R33*l6 + 1.0;

		//// complementarity equation in Friction Model
		F[15] = mu*mu*p_n*p_n - p_r*p_r/(e_r*e_r) - p_t*p_t/(e_t*e_t) - p_o*p_o/(e_o*e_o);

		//// complementarity equations in contact constraints
		F[16] = len/2.0 + R11*a1_x + R21*a1_y + R31*a1_z - R11*q_x - R21*q_y - R31*q_z;
		F[17] = len/2.0 - R11*a1_x - R21*a1_y - R31*a1_z + R11*q_x + R21*q_y + R31*q_z;
		F[18] = wid/2.0 + R12*a1_x + R22*a1_y + R32*a1_z - R12*q_x - R22*q_y - R32*q_z;
		F[19] = wid/2.0 - R12*a1_x - R22*a1_y - R32*a1_z + R12*q_x + R22*q_y + R32*q_z;
		F[20] = heg/2.0 + R13*a1_x + R23*a1_y + R33*a1_z - R13*q_x - R23*q_y - R33*q_z;
		F[21] = heg/2.0 - R13*a1_x - R23*a1_y - R33*a1_z + R13*q_x + R23*q_y + R33*q_z;
		F[22] = -a2_z;

		//// non-penetration
		F[23] = a1_z;

		return 0;
	}

	int Simulation::PathSim::cubeJacEval(double* z, int* column_starting_indices,
		int* len_of_each_column, int* row_index_of_each_value, double* values)
	{
		double& v_x = z[0];
		double& v_y = z[1];
		double& v_z = z[2];
		double& w_x = z[3];
		double& w_y = z[4];
		double& w_z = z[5];

		double& a1_x = z[6];
		double& a1_y = z[7];
		double& a1_z = z[8];

		double& a2_x = z[9];
		double& a2_y = z[10];
		double& a2_z = z[11];

		double& p_t = z[12];
		double& p_o = z[13];
		double& p_r = z[14];

		double& sig = z[15];

		double& l1 = z[16];
		double& l2 = z[17];
		double& l3 = z[18];
		double& l4 = z[19];
		double& l5 = z[20];
		double& l6 = z[21];
		double& l7 = z[22];

		double& p_n = z[23];

		double q0 = -(2.0*((h*q1_o*w_x)/2.0 - q0_o + (h*q2_o*w_y)/2.0 + (h*q3_o*w_z)/2.0))/std::sqrt(h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4.0);
		double q1 = (2.0*(q1_o + (h*q0_o*w_x)/2.0 + (h*q3_o*w_y)/2.0 - (h*q2_o*w_z)/2.0))/std::sqrt(h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4.0);
		double q2 = (2.0*(q2_o - (h*q3_o*w_x)/2.0 + (h*q0_o*w_y)/2.0 + (h*q1_o*w_z)/2.0))/std::sqrt(h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4.0);
		double q3 = (2.0*(q3_o + (h*q2_o*w_x)/2.0 - (h*q1_o*w_y)/2.0 + (h*q0_o*w_z)/2.0))/std::sqrt(h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4.0);

		double q_x = q_xo+h*v_x;
		double q_y = q_yo+h*v_y;
		double q_z = q_zo+h*v_z;

		double R11 = q0*q0 + q1*q1 - q2*q2 - q3*q3;
		double R12 = 2*q1*q2 - 2*q0*q3;
		double R13 = 2*q0*q2 + 2*q1*q3;
		double R21 = 2*q0*q3 + 2*q1*q2;
		double R22 = q0*q0 - q1*q1 + q2*q2 - q3*q3;
		double R23 = 2*q2*q3 - 2*q0*q1;
		double R31 = 2*q1*q3 - 2*q0*q2;
		double R32 = 2*q0*q1 + 2*q2*q3;
		double R33 = q0*q0 - q1*q1 - q2*q2 + q3*q3;

		double I11 = I_xx*R11*R11 + I_yy*R12*R12 + I_zz*R13*R13;
		double I12 = I_xx*R21*R11 + I_yy* R22*R12 + I_zz*R23*R13;
		double I13 = I_xx*R31*R11 + I_yy* R32*R12 + I_zz*R33*R13;
		double I21 = I12;
		double I22 = I_xx*R21*R21 + I_yy*R22*R22 + I_zz*R23*R23;
		double I23 = I_xx*R31*R21 + I_yy*R32*R22 + I_zz*R33*R23;
		double I31 = I13;
		double I32 = I23;
		double I33 = I_xx*R31*R31 + I_yy*R32*R32 + I_zz*R33*R33;

		// Newton_Euler
		Eigen::MatrixXd J1 = Eigen::MatrixXd::Zero(24, 45);
		J1.block<3, 3>(0, 0) = -m*Eigen::Matrix3d::Identity();
		J1.block<2, 2>(0, 12).setIdentity();
		J1(2, 23) = 1.0;
		J1.block<3, 3>(3, 3) << -I11, -I12, -I13, -I21, -I22, -I23, -I31, -I32, -I33;
		J1.block<3, 3>(3, 6) << 0.0, p_n, -p_o, -p_n, 0.0, p_t, p_o, -p_t, 0.0;
		J1.block<3, 2>(3, 12) << 0.0, q_z-a1_z, a1_z-q_z, 0.0, q_y-a1_y, a1_x-q_x;
		J1.col(23).segment(3, 2) << a1_y-q_y, q_x-a1_x;
		J1(5, 14) = 1.0;
		J1.block<3, 9>(3, 33) << w_xo - w_x, w_yo - w_y, w_zo - w_z, h*w_xo*w_zo, h*w_yo*w_zo, h*w_zo*w_zo, -h*w_xo*w_yo, -h*w_yo*w_yo, -h*w_yo*w_zo,
			-h*w_xo*w_zo, -h*w_yo*w_zo, -h*w_zo*w_zo, w_xo - w_x, w_yo - w_y, w_zo - w_z, h*w_xo*w_xo, h*w_xo*w_yo, h*w_xo*w_zo,
			h*w_xo*w_yo, h*w_yo*w_yo, h*w_yo*w_zo, -h*w_xo*w_xo, -h*w_xo*w_yo, -h*w_xo*w_zo, w_xo - w_x, w_yo - w_y, w_zo - w_z;
		J1.block<3, 3>(3, 42) << 0.0, -p_n, p_o, p_n, 0.0, -p_t, -p_o, p_t, 0.0;
		//	// contact constraints without complementarity equation
		J1(6, 0) = e_t*e_t*mu*p_n;
		J1(7, 1) = e_o*e_o*mu*p_n;
		J1.block<3, 3>(6, 3) << 0.0, e_t*e_t*mu*p_n*(a1_z - q_z), -e_t*e_t*mu*p_n*(a1_y - q_y),
			-e_o*e_o*mu*p_n*(a1_z - q_z), 0.0, e_o*e_o*mu*p_n*(a1_x - q_x),
			0.0, 0.0, e_r*e_r*mu*p_n;
		J1.block<2, 3>(6, 6) << 0.0, -e_t*e_t*mu*p_n*w_z, e_t*e_t*mu*p_n*w_y,
			e_o*e_o*mu*p_n*w_z, 0.0, -e_o*e_o*mu*p_n*w_x;
		J1.block<3, 3>(6, 12) = sig*Eigen::Matrix3d::Identity();
		J1.col(15).segment(6, 3) << p_t, p_o, p_r;
		J1.col(23).segment(6, 3) << e_t*e_t*mu*v_x - e_t*e_t*mu*w_z*(a1_y - q_y) + e_t*e_t*mu*w_y*(a1_z - q_z),
			e_o*e_o*mu*v_y + e_o*e_o*mu*w_z*(a1_x - q_x) - e_o*e_o*mu*w_x*(a1_z - q_z),
			e_r*e_r*mu*w_z;
		J1.block<2, 3>(6, 42) << 0.0, e_t*e_t*mu*p_n*w_z, -e_t*e_t*mu*p_n*w_y,
			-e_o*e_o*mu*p_n*w_z, 0.0, e_o*e_o*mu*p_n*w_x;

		//	// contact constraints without complementarity equation
		J1.block<3, 3>(9, 6) = -Eigen::Matrix3d::Identity();
		J1.block<3, 3>(9, 9).setIdentity();
		J1(11, 22) = 1.0;

		J1.block<3, 6>(12, 16) << -R11, R11, -R12, R12, -R13, R13,
			-R21, R21, -R22, R22, -R23, R23,
			-R31, R31, -R32, R32, -R33, R33;
		J1.row(12).segment(24, 3) << l2 - l1, l4 - l3, l6 - l5;
		J1.row(13).segment(27, 3) << l2 - l1, l4 - l3, l6 - l5;
		J1.row(14).segment(30, 3) << l2 - l1, l4 - l3, l6 - l5;
		//	// complementarity equation in Friction Model
		J1.row(15).segment(12, 3) << -(2*p_t)/(e_t*e_t), -(2*p_o)/(e_o*e_o), -(2*p_r)/(e_r*e_r);
		J1(15, 23) = 2*mu*mu*p_n;

		//// complementarity equations in contact constraints
		J1.block<6, 3>(16, 6) << R11, R21, R31,
			-R11, -R21, -R31,
			R12, R22, R32,
			-R12, -R22, -R32,
			R13, R23, R33,
			-R13, -R23, -R33;
		J1.block<6, 9>(16, 24) <<
			a1_x - q_x, 0.0, 0.0, a1_y - q_y, 0.0, 0.0, a1_z - q_z, 0.0, 0.0,
			q_x - a1_x, 0.0, 0.0, q_y - a1_y, 0.0, 0.0, q_z - a1_z, 0.0, 0.0,
			0.0, a1_x - q_x, 0.0, 0.0, a1_y - q_y, 0.0, 0.0, a1_z - q_z, 0.0,
			0.0, q_x - a1_x, 0.0, 0.0, q_y - a1_y, 0.0, 0.0, q_z - a1_z, 0.0,
			0.0, 0.0, a1_x - q_x, 0.0, 0.0, a1_y - q_y, 0.0, 0.0, a1_z - q_z,
			0.0, 0.0, q_x - a1_x, 0.0, 0.0, q_y - a1_y, 0.0, 0.0, q_z - a1_z;

		J1.block<6, 3>(16, 42) << -R11, -R21, -R31,
			R11, R21, R31,
			-R12, -R22, -R32,
			R12, R22, R32,
			-R13, -R23, -R33,
			R13, R23, R33;
		J1(22, 11) = -1.0;
		J1(23, 8) = 1.0;

		Eigen::MatrixXd J2 = Eigen::MatrixXd::Zero(45, 28);
		J2.topLeftCorner(24, 24).setIdentity();
		J2.block<9, 4>(24, 24) << 2*q0, 2*q1, -2*q2, -2*q3,
			-2*q3, 2*q2, 2*q1, -2*q0,
			2*q2, 2*q3, 2*q0, 2*q1,
			2*q3, 2*q2, 2*q1, 2*q0,
			2*q0, -2*q1, 2*q2, -2*q3,
			-2*q1, -2*q0, 2*q3, 2*q2,
			-2*q2, 2*q3, -2*q0, 2*q1,
			2*q1, 2*q0, 2*q3, 2*q2,
			2*q0, -2*q1, -2*q2, 2*q3;

		J2.block<9, 4>(33, 24) << 
			4*I_xx*R11*q0 - 4*I_yy*R12*q3 + 4*I_zz*R13*q2,													4*I_xx*R11*q1 + 4*I_yy*R12*q2 + 4*I_zz*R13*q3,													4*I_yy*R12*q1 - 4*I_xx*R11*q2 + 4*I_zz*R13*q0,													4*I_zz*R13*q1 - 4*I_yy*R12*q0 - 4*I_xx*R11*q3,
			2*I_xx*R11*q3 + 2*I_xx*R21*q0 + 2*I_yy*R12*q0 - 2*I_yy*R22*q3 - 2*I_zz*R13*q1 + 2*I_zz*R23*q2,	2*I_xx*R11*q2 + 2*I_xx*R21*q1 - 2*I_yy*R12*q1 + 2*I_yy*R22*q2 - 2*I_zz*R13*q0 + 2*I_zz*R23*q3,	2*I_xx*R11*q1 - 2*I_xx*R21*q2 + 2*I_yy*R12*q2 + 2*I_yy*R22*q1 + 2*I_zz*R13*q3 + 2*I_zz*R23*q0,	2*I_xx*R11*q0 - 2*I_xx*R21*q3 - 2*I_yy*R12*q3 - 2*I_yy*R22*q0 + 2*I_zz*R13*q2 + 2*I_zz*R23*q1,
			2*I_xx*R31*q0 - 2*I_xx*R11*q2 + 2*I_yy*R12*q1 - 2*I_yy*R32*q3 + 2*I_zz*R13*q0 + 2*I_zz*R33*q2,	2*I_xx*R11*q3 + 2*I_xx*R31*q1 + 2*I_yy*R12*q0 + 2*I_yy*R32*q2 - 2*I_zz*R13*q1 + 2*I_zz*R33*q3,	2*I_yy*R12*q3 - 2*I_xx*R31*q2 - 2*I_xx*R11*q0 + 2*I_yy*R32*q1 - 2*I_zz*R13*q2 + 2*I_zz*R33*q0,	2*I_xx*R11*q1 - 2*I_xx*R31*q3 + 2*I_yy*R12*q2 - 2*I_yy*R32*q0 + 2*I_zz*R13*q3 + 2*I_zz*R33*q1,
			2*I_xx*R11*q3 + 2*I_xx*R21*q0 + 2*I_yy*R12*q0 - 2*I_yy*R22*q3 - 2*I_zz*R13*q1 + 2*I_zz*R23*q2,	2*I_xx*R11*q2 + 2*I_xx*R21*q1 - 2*I_yy*R12*q1 + 2*I_yy*R22*q2 - 2*I_zz*R13*q0 + 2*I_zz*R23*q3,	2*I_xx*R11*q1 - 2*I_xx*R21*q2 + 2*I_yy*R12*q2 + 2*I_yy*R22*q1 + 2*I_zz*R13*q3 + 2*I_zz*R23*q0,	2*I_xx*R11*q0 - 2*I_xx*R21*q3 - 2*I_yy*R12*q3 - 2*I_yy*R22*q0 + 2*I_zz*R13*q2 + 2*I_zz*R23*q1,
			4*I_xx*R21*q3 + 4*I_yy*R22*q0 - 4*I_zz*R23*q1,													4*I_xx*R21*q2 - 4*I_yy*R22*q1 - 4*I_zz*R23*q0,													4*I_xx*R21*q1 + 4*I_yy*R22*q2 + 4*I_zz*R23*q3,													4*I_xx*R21*q0 - 4*I_yy*R22*q3 + 4*I_zz*R23*q2,
			2*I_xx*R31*q3 - 2*I_xx*R21*q2 + 2*I_yy*R22*q1 + 2*I_yy*R32*q0 + 2*I_zz*R23*q0 - 2*I_zz*R33*q1,	2*I_xx*R21*q3 + 2*I_xx*R31*q2 + 2*I_yy*R22*q0 - 2*I_yy*R32*q1 - 2*I_zz*R23*q1 - 2*I_zz*R33*q0,	2*I_xx*R31*q1 - 2*I_xx*R21*q0 + 2*I_yy*R22*q3 + 2*I_yy*R32*q2 - 2*I_zz*R23*q2 + 2*I_zz*R33*q3,	2*I_xx*R21*q1 + 2*I_xx*R31*q0 + 2*I_yy*R22*q2 - 2*I_yy*R32*q3 + 2*I_zz*R23*q3 + 2*I_zz*R33*q2,
			2*I_xx*R31*q0 - 2*I_xx*R11*q2 + 2*I_yy*R12*q1 - 2*I_yy*R32*q3 + 2*I_zz*R13*q0 + 2*I_zz*R33*q2,	2*I_xx*R11*q3 + 2*I_xx*R31*q1 + 2*I_yy*R12*q0 + 2*I_yy*R32*q2 - 2*I_zz*R13*q1 + 2*I_zz*R33*q3,	2*I_yy*R12*q3 - 2*I_xx*R31*q2 - 2*I_xx*R11*q0 + 2*I_yy*R32*q1 - 2*I_zz*R13*q2 + 2*I_zz*R33*q0,	2*I_xx*R11*q1 - 2*I_xx*R31*q3 + 2*I_yy*R12*q2 - 2*I_yy*R32*q0 + 2*I_zz*R13*q3 + 2*I_zz*R33*q1,
			2*I_xx*R31*q3 - 2*I_xx*R21*q2 + 2*I_yy*R22*q1 + 2*I_yy*R32*q0 + 2*I_zz*R23*q0 - 2*I_zz*R33*q1,	2*I_xx*R21*q3 + 2*I_xx*R31*q2 + 2*I_yy*R22*q0 - 2*I_yy*R32*q1 - 2*I_zz*R23*q1 - 2*I_zz*R33*q0,	2*I_xx*R31*q1 - 2*I_xx*R21*q0 + 2*I_yy*R22*q3 + 2*I_yy*R32*q2 - 2*I_zz*R23*q2 + 2*I_zz*R33*q3,	2*I_xx*R21*q1 + 2*I_xx*R31*q0 + 2*I_yy*R22*q2 - 2*I_yy*R32*q3 + 2*I_zz*R23*q3 + 2*I_zz*R33*q2,
			4*I_yy*R32*q1 - 4*I_xx*R31*q2 + 4*I_zz*R33*q0,													4*I_xx*R31*q3 + 4*I_yy*R32*q0 - 4*I_zz*R33*q1,													4*I_yy*R32*q3 - 4*I_xx*R31*q0 - 4*I_zz*R33*q2,													4*I_xx*R31*q1 + 4*I_yy*R32*q2 + 4*I_zz*R33*q3;

		J2.block<3, 3>(42, 0) = h*Eigen::Matrix3d::Identity();

		Eigen::MatrixXd J3 = Eigen::MatrixXd::Zero(28, 24);
		J3.topRows(24).setIdentity();
		double N_J3 = std::pow(h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4, 1.5);
		J3.block<4, 3>(24, 3) << 
			-(h*(q1_o*h*h*w_y*w_y - q2_o*w_x*h*h*w_y + q1_o*h*h*w_z*w_z - q3_o*w_x*h*h*w_z + 2*q0_o*w_x*h + 4*q1_o))/N_J3, -(h*(q2_o*h*h*w_x*w_x - q1_o*w_y*h*h*w_x + q2_o*h*h*w_z*w_z - q3_o*w_y*h*h*w_z + 2*q0_o*w_y*h + 4*q2_o))/N_J3, -(h*(q3_o*h*h*w_x*w_x - q1_o*w_z*h*h*w_x + q3_o*h*h*w_y*w_y - q2_o*w_z*h*h*w_y + 2*q0_o*w_z*h + 4*q3_o))/N_J3,
			 (h*(q0_o*h*h*w_y*w_y - q3_o*w_x*h*h*w_y + q0_o*h*h*w_z*w_z + q2_o*w_x*h*h*w_z - 2*q1_o*w_x*h + 4*q0_o))/N_J3,  (h*(q3_o*h*h*w_x*w_x - q0_o*w_y*h*h*w_x + q3_o*h*h*w_z*w_z + q2_o*w_y*h*h*w_z - 2*q1_o*w_y*h + 4*q3_o))/N_J3, -(h*(q2_o*h*h*w_x*w_x + q0_o*w_z*h*h*w_x + q2_o*h*h*w_y*w_y + q3_o*w_z*h*h*w_y + 2*q1_o*w_z*h + 4*q2_o))/N_J3,
			-(h*(q3_o*h*h*w_y*w_y + q0_o*w_x*h*h*w_y + q3_o*h*h*w_z*w_z + q1_o*w_x*h*h*w_z + 2*q2_o*w_x*h + 4*q3_o))/N_J3,  (h*(q0_o*h*h*w_x*w_x + q3_o*w_y*h*h*w_x + q0_o*h*h*w_z*w_z - q1_o*w_y*h*h*w_z - 2*q2_o*w_y*h + 4*q0_o))/N_J3,  (h*(q1_o*h*h*w_x*w_x + q3_o*w_z*h*h*w_x + q1_o*h*h*w_y*w_y - q0_o*w_z*h*h*w_y - 2*q2_o*w_z*h + 4*q1_o))/N_J3,
			 (h*(q2_o*h*h*w_y*w_y + q1_o*w_x*h*h*w_y + q2_o*h*h*w_z*w_z - q0_o*w_x*h*h*w_z - 2*q3_o*w_x*h + 4*q2_o))/N_J3, -(h*(q1_o*h*h*w_x*w_x + q2_o*w_y*h*h*w_x + q1_o*h*h*w_z*w_z + q0_o*w_y*h*h*w_z + 2*q3_o*w_y*h + 4*q1_o))/N_J3,  (h*(q0_o*h*h*w_x*w_x - q2_o*w_z*h*h*w_x + q0_o*h*h*w_y*w_y + q1_o*w_z*h*h*w_y - 2*q3_o*w_z*h + 4*q0_o))/N_J3;

		Eigen::SparseMatrix<double> J = Eigen::SparseView<Eigen::MatrixXd>(J1*J2*J3, 1.0).cast<double>();
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

	// Uses PATH variables, capture from target entity first if necessary
	// Adds step to end of timeline and returns true if successful
	// Does not update target entity
	bool Simulation::PathSim::addStep(Simulation::Timeline& timeline)
	{
		pathMain(n, nnz, &status, z, F, lower_bounds, upper_bounds);

		printZ();

		if (status != 1)
			return false;

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

	void Simulation::PathSim::captureBoxState()
	{
		// cache
		q_xo = box->getPosition().x;
		q_yo = box->getPosition().y;
		q_zo = box->getPosition().z;
		q0_o = box->getOrientation().w;
		q1_o = box->getOrientation().x;
		q2_o = box->getOrientation().y;
		q3_o = box->getOrientation().z;
		v_xo = box->getVelocity().x;
		v_yo = box->getVelocity().y;
		v_zo = box->getVelocity().z;
		w_xo = box->getAngVelocity().x;
		w_yo = box->getAngVelocity().y;
		w_zo = box->getAngVelocity().z;

		// variables
		z[0]  = v_xo;
		z[1]  = v_yo;
		z[2]  = v_zo;
		z[3]  = w_xo;
		z[4]  = w_yo;
		z[5]  = w_zo;
		z[6]  = q_xo;
		z[7]  = q_yo;
		z[8]  = q_zo - heg/2;
		z[9]  = q_xo;
		z[10] = q_yo;
		z[11] = core::FLOOR_Z;
	}

	void Simulation::PathSim::printZ()
	{
		std::cout << "---Sam Tan's output---\nStatus: " << status << "\n          Z:\n-----------------------";
		std::cout <<
			"\n0 | v_x  | " << z[0]  <<
			"\n1 | v_y  | " << z[1]  <<
			"\n2 | v_z  | " << z[2]  <<
			"\n3 | w_x  | " << z[3]  <<
			"\n4 | w_y  | " << z[4]  <<
			"\n5 | w_z  | " << z[5]  <<
			"\n6 | a1_x | " << z[6]  <<
			"\n7 | a1_y | " << z[7]  <<
			"\n8 | a1_z | " << z[8]  <<
			"\n9 | a2_x | " << z[9]  <<
			"\n10| a2_y | " << z[10] <<
			"\n11| a2_z | " << z[11] <<
			"\n12| p_t  | " << z[12] <<
			"\n13| p_o  | " << z[13] <<
			"\n14| p_r  | " << z[14] <<
			"\n15| sig  | " << z[15] <<
			"\n16| l1   | " << z[16] <<
			"\n17| l2   | " << z[17] <<
			"\n18| l3   | " << z[18] <<
			"\n19| l4   | " << z[19] <<
			"\n20| l5   | " << z[20] <<
			"\n21| l6   | " << z[21] <<
			"\n22| l7   | " << z[22] <<
			"\n23| p_n  | " << z[23] << std::endl;
	}

	void Simulation::PathSim::printCache()
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

	void Simulation::PathSim::printConstants()
	{
		std::cout << "\nConstants:" <<
			"\nm    | " << m    <<
			"\nmu   | " << mu   <<
			"\nI_xx | " << I_xx <<
			"\nI_yy | " << I_yy <<
			"\nI_zz | " << I_zz <<
			"\ne_o  | " << e_o  <<
			"\ne_r  | " << e_r  <<
			"\ne_t  | " << e_t  <<
			"\nlen  | " << len  <<
			"\nwid  | " << wid  <<
			"\nheg  | " << heg  << std::endl;
	}

	int Simulation::addSteps(Simulation::Timeline& timeline, int numSteps)
	{
		// start from current state, replacing any existing version of events
		timeline.erase(timeline.begin() + parameters.currentStep, timeline.end());
		recordTimestep(pathSim.box);

		int successes = 0;
		bool usePath = false; // start with freefall since it's fast to compute
		bool lastPathSuccessful = true;
		bool lastFreefallSuccessful = true;
		while (successes < numSteps)
		{
			if (usePath)
			{
				// try a path step
				pathSim.box->loadState(timeline.back());
				pathSim.captureBoxState();
				pathSim.printZ();
				pathSim.printCache();
				if (pathSim.addStep(timeline))
				{
					lastPathSuccessful = true;
					++successes;
				}
				else
				{
					// path not successful
					lastPathSuccessful = false;
					if (lastFreefallSuccessful)
						usePath = false; // switch to freefall
					else
						return successes; // neither method is working
				}
			}
			else
			{
				// try a freefall step
				if (addFreefallStep(pathSim.box))
				{
					lastFreefallSuccessful = true;
					++successes;
				}
				else
				{
					// freefall not successful
					lastFreefallSuccessful = false;
					if (lastPathSuccessful)
						usePath = true; // switch to path
					else
						return successes; // neither method is working
				}
			}
		}
		return successes;
	}

	int Simulation::addStepsUntilEnd(Simulation::Timeline & timeline)
	{
		static constexpr int max = 1000;

		// start from current state, replacing any existing version of events
		timeline.erase(timeline.begin() + parameters.currentStep, timeline.end());
		recordTimestep(pathSim.box);

		int successes = 0;
		bool usePath = false; // start with freefall since it's fast to compute
		bool lastPathSuccessful = true;
		bool lastFreefallSuccessful = true;

		while (true)
		{
			if (usePath)
			{
				// try a path step
				pathSim.box->loadState(timeline.back());
				pathSim.captureBoxState();
				pathSim.printZ();
				pathSim.printCache();
				if (pathSim.addStep(timeline))
				{
					lastPathSuccessful = true;
					++successes;
				}
				else
				{
					// path not successful
					lastPathSuccessful = false;
					if (lastFreefallSuccessful)
						usePath = false; // switch to freefall
					else
						return successes; // neither method is working
				}
			}
			else
			{
				// try a freefall step
				if (addFreefallStep(pathSim.box))
				{
					lastFreefallSuccessful = true;
					++successes;
				}
				else
				{
					// freefall not successful
					lastFreefallSuccessful = false;
					if (lastPathSuccessful)
						usePath = true; // switch to path
					else
						return successes; // neither method is working
				}
			}

			if (glm::length2(timeline.back().velocity) < 1e-6f &&
				glm::length2(timeline.back().angVelocity) < 1e-6f)
				return successes; // everything stopped, we're done

			if (successes > max)
				return successes;
		}
		return successes;
	}

	// Does not record contact point
	void Simulation::recordTimestep(entity::Entity* e)
	{
		timeline.emplace_back(
			e->getPosition(),
			e->getOrientation(),
			e->getVelocity(),
			e->getAngVelocity(),
			e->getPosition()
		);
	}

	// Starts from end of timeline
	// Adds result to end of timeline and returns true if successful
	// Updates target entity to end of timeline
	bool Simulation::addFreefallStep(entity::Entity* e)
	{
		e->loadState(timeline.back());
		recordTimestep(e);

		e->update();
		if (e->intersectsFloor())
		{
			// Not gonna work, undo
			e->loadState(timeline.back());
			timeline.pop_back();
			return false;
		}
		else
		{
			// it's good
			timeline.pop_back();
			recordTimestep(e);
			return true;
		}
	}

	Simulation::~Simulation()
	{
		for (entity::Entity* e : entities)
			delete e;
	}

	void Simulation::update()
	{
		static auto prevTime = std::chrono::steady_clock::now();
		static int prevStep = parameters.currentStep;
		if (!parameters.timePaused)
		{
			// step time forward by however much the last frame corresponds to
			auto now = std::chrono::steady_clock::now();
			auto diff = std::chrono::duration_cast<std::chrono::microseconds>(now - prevTime).count();
			float timeDiff = diff*1e-6f*parameters.playbackSpeed;

			// last calculated time in the timeline. Used for clamping, below
			float backTime = timeline.backTime();

			// if the current step and current time don't match
			if (std::copysignf(1.0f, parameters.playbackSpeed) * (parameters.playbackTime / core::TIME_STEP - parameters.currentStep) >= 1.0)
			{
				// Check to see if it's just because we reached the end of the timeline
				if (parameters.playbackTime < 0.0f)
				{
					if (parameters.loopPlayback)
						parameters.playbackTime = std::fmod(parameters.playbackTime,
							backTime + core::TIME_STEP) + backTime + core::TIME_STEP;
					else
						parameters.playbackTime = 0.0f;
				}
				if (parameters.playbackTime >= backTime + core::TIME_STEP)
				{
					if (parameters.loopPlayback)
						parameters.playbackTime = std::fmod(parameters.playbackTime,
							backTime + core::TIME_STEP);
					else
						parameters.playbackTime = backTime;
				}

				// User must have changed the currentStep slider through ImGui. 
				// Update playbackTime accordingly:
				parameters.playbackTime = parameters.currentStep * core::TIME_STEP;
			}

			parameters.playbackTime += timeDiff; // advance time

			// clamp
			if (parameters.playbackTime < 0.0f)
			{
				if (parameters.loopPlayback)
					parameters.playbackTime = std::fmod(parameters.playbackTime, 
						backTime + core::TIME_STEP) + backTime + core::TIME_STEP;
				else
					parameters.playbackTime = 0.0f;
			}
			if (parameters.playbackTime >= backTime + core::TIME_STEP)
			{
				if (parameters.loopPlayback)
					parameters.playbackTime = std::fmod(parameters.playbackTime, 
						backTime + core::TIME_STEP);
				else
					parameters.playbackTime = backTime;
			}

			// Now find the step number
			int step = static_cast<int>(parameters.playbackTime/core::TIME_STEP);

			// Clamping again just in case. It do be like that
			step = std::max(step, 0);
			step = std::min(step, static_cast<int>(timeline.size())-1);

			// do it
			parameters.currentStep = step;
		}
		if (prevStep != parameters.currentStep)
			pathSim.box->loadState(timeline[parameters.currentStep]);
		prevStep = parameters.currentStep;
		prevTime = std::chrono::steady_clock::now();
	}

	void Simulation::render(graphics::Renderer& renderer, const glm::vec3& cameraPos) const
	{
		if (cameraPos.z > core::FLOOR_Z)
		{
			if (parameters.showEnvironment)
				renderEnvironment(renderer);
			renderEntities(renderer, cameraPos);
		}
		else // reverse the rendering order
		{
			renderEntities(renderer, cameraPos);
			if (parameters.showEnvironment)
				renderEnvironment(renderer);
		}


	}

	void Simulation::renderGUI(const graphics::Camera& camera)
	{
		// FPS calculations
		static auto previous = std::chrono::steady_clock::now();

		static unsigned int frameCount = 0u;
		frameCount++;

		static float fps = 0.0;
		if (frameCount >= FPS_TRACKER_SMOOTHING)
		{
			auto now = std::chrono::steady_clock::now();
			auto diff = std::chrono::duration_cast<std::chrono::microseconds>(now - previous).count();
			previous = std::chrono::steady_clock::now();

			fps = 1.0e6f*frameCount / diff;
			frameCount = 0u;
		}

		static int calculateStepsNum = 100;

		if (ImGui::Begin("Simulation"))
		{
			// Parameter checkboxes
			{
				//ImGui::Checkbox("Enable Gravity", &(parameters.gravityEnabled));
				ImGui::Checkbox("Pause Time", &(parameters.timePaused));
				ImGui::Checkbox("Loop Playback", &(parameters.loopPlayback));
			}
			// Playback controls
			{
				if (ImGui::Button("Once"))
				{
					parameters.timePaused = false;
					parameters.loopPlayback = false;
					parameters.playbackSpeed = 1.0f;
					parameters.currentStep = 0;
				}
				ImGui::SameLine();
				if (ImGui::Button("Loop"))
				{
					parameters.timePaused = false;
					parameters.loopPlayback = true;
					parameters.playbackSpeed = 1.0f;
					parameters.currentStep = 0;
				}
				ImGui::SameLine();
				if (ImGui::Button("Reset"))
				{
					parameters.timePaused = true;
					parameters.currentStep = 0;
				}
				ImGui::Text("Playback Speed:");
				ImGui::InputFloat("##PlaybackSpeedNum", &(parameters.playbackSpeed));
				if (ImGui::IsItemHovered())
				{
					static constexpr float factor = 0.1f;
					float scr = ImGui::GetIO().MouseWheel;
					parameters.playbackSpeed += scr * factor;
				}
				ImGui::Text("Elapsed Time: %.3f s", timeline.timeOf(parameters.currentStep));
			}
			// Timestep slider
			{

				ImGui::Text("Current Timestep:");
				std::string numStepsString("%d / ");
				numStepsString += std::to_string(timeline.size()-1);
				ImGui::BeginGroup();
				ImGui::SliderInt("##CurrentTimestepNum", &(parameters.currentStep), 0, 
					static_cast<int>(timeline.size())-1, numStepsString.c_str());

				// Fine adjustment buttons
				ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
				static const float buttonSize = ImGui::GetFrameHeight();
				static const ImVec2 buttonSizeV = ImVec2(buttonSize, buttonSize);
				ImGui::PushButtonRepeat(true);
				if (ImGui::Button("-##Timestep", buttonSizeV)
					&& parameters.currentStep > 0)
					--parameters.currentStep;
				ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
				if (ImGui::Button("+##Timestep", buttonSizeV)
					&& parameters.currentStep < static_cast<int>(timeline.size()) - 1)
					++parameters.currentStep;
				ImGui::PopButtonRepeat();
				ImGui::EndGroup();

				// Can use mouse wheel to scroll timeline
				if (ImGui::IsItemHovered())
				{
					int scr = static_cast<int>(ImGui::GetIO().MouseWheel);
					parameters.currentStep -= scr;
					if (scr != 0)
						parameters.timePaused = true;
					
					// now clamp
					if (parameters.currentStep >= static_cast<int>(timeline.size()))
						parameters.currentStep = timeline.size() - 1;
					else if (parameters.currentStep < 0)
						parameters.currentStep = 0;
				}
			}
			// Timestep calculation control
			{
				ImGui::Text("Calculate Steps:");
				ImGui::InputInt("##CalculateStepsNum", &calculateStepsNum);
				if (ImGui::IsItemHovered())
					calculateStepsNum += static_cast<int>(ImGui::GetIO().MouseWheel);
				if (calculateStepsNum < 0)
					calculateStepsNum = 0;
				ImGui::SameLine();
				if (ImGui::ArrowButton("##CalculateStepsButton", ImGuiDir_Right))
				{
					addSteps(timeline, calculateStepsNum);
					parameters.timePaused = false;
					parameters.loopPlayback = false;
				}
				if (ImGui::Button("Calculate Until Stop"))
				{
					addStepsUntilEnd(timeline);
					parameters.timePaused = false;
					parameters.loopPlayback = false;
				}
			}
			{
				ImGui::Separator();
				ImGui::Text("FPS: %.0f", fps);
			}
			// Visual controls
			ImGui::Separator();
			{
				ImGui::Checkbox("Show Environment", &(parameters.showEnvironment));

				ImGui::Indent(); // showShadows should be greyed out if !showEnvironment
				{
					if (!parameters.showEnvironment)
					{
						ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
						ImGui::PushStyleColor(ImGuiCol_CheckMark, ImGui::GetStyle().Colors[ImGuiCol_FrameBg]);
						ImGui::InvisibleButton("##ShowShadowsClickInterceptor", ImVec2(110.0f, ImGui::GetFrameHeight()));
						ImGui::SameLine(ImGui::GetStyle().IndentSpacing +
							ImGui::GetStyle().WindowPadding.x, 0.0f);
					}

					ImGui::Checkbox("Show Shadows", &(parameters.showShadows));

					if (!parameters.showEnvironment)
						ImGui::PopStyleColor(2);
				}
				ImGui::Unindent();
				
				ImGui::Checkbox("Show Contact Point", &(parameters.showContactPoint));
			}
			// Entity checkboxes
			ImGui::Separator();
			{
				ImGui::Text("Show Entity Information:");
				{
					ImGui::BeginChild("Entity Scroll Pane");
					for (entity::Entity* e : entities)
					{
						ImGui::Checkbox(e->getName().c_str(), &(e->shouldShow.gui));
					}
					ImGui::EndChild();
				}
			}
		}
		ImGui::End();

		for (entity::Entity* e : entities)
		{
			if (e->shouldShow.gui)
				e->renderGUI();
			if(e->shouldShow.label)
				e->renderLabel(camera);
		}
	}

	void Simulation::renderGUIOverlay(graphics::Renderer& renderer, const graphics::Camera& camera) const
	{
		for (entity::Entity* e : entities)
		{
			if (e->shouldShow.velocityVector)
				e->renderVelocityVector(renderer, camera);
			if (e->shouldShow.angVelocityVector)
				e->renderAngularVelocity(renderer, camera);
			if (e->shouldShow.positionMarker)
				e->renderPositionMarker(renderer, camera);
		}

		if (parameters.showContactPoint)
			renderContactPoint(renderer, camera);
	}

	void Simulation::add(entity::Entity* e)
	{
		entities.push_back(e);

		if (timeline.empty())
		{
			// initialize timeline
			recordTimestep(e);
		}
	}

	const glm::vec3 & Simulation::getFocusedEntityPosition() const
	{
		return entities[0]->getPosition();
	}

	void Simulation::renderEntities(graphics::Renderer& renderer, const glm::vec3& cameraPos) const
	{
		for (entity::Entity* e : entities)
		{
			e->render(renderer);
			if (parameters.showShadows && parameters.showEnvironment)
				e->renderShadow(renderer, cameraPos);
		}
	}

	void Simulation::renderEnvironment(graphics::Renderer& renderer) const
	{
		// floor
		//renderer.submit(environment.floor);
		glm::vec4 squareColor;
		float floorHalfwidth = 10.0f;
		float floorCheckerSize = 1.0f;
		for (float i = -floorHalfwidth; i < floorHalfwidth; i += floorCheckerSize)
		{
			for (float j = -floorHalfwidth; j < floorHalfwidth; j += floorCheckerSize)
			{
				if (std::abs(std::fmodf(i+j, 2.0f)) < 1e-3)
					squareColor = graphics::COLOR_WHITE;
				else
					squareColor = glm::vec4(0.6f, 0.6f, 0.6f, 0.5f); // gray

				squareColor.a = 0.5f;

				renderer.submit(graphics::Quad {{
					{ squareColor, glm::vec3(i, j, core::FLOOR_Z) },
					{ squareColor, glm::vec3(i, j+floorCheckerSize, core::FLOOR_Z) },
					{ squareColor, glm::vec3(i+floorCheckerSize, j+floorCheckerSize, core::FLOOR_Z) },
					{ squareColor, glm::vec3(i+floorCheckerSize, j,core::FLOOR_Z) }
				}});
			}
		}
	}

	void Simulation::renderContactPoint(graphics::Renderer& renderer, const graphics::Camera& camera) const
	{
		static constexpr float radius = 0.04f;

		const Timestep& now = timeline[parameters.currentStep];

		if (!now.hasContactPoint())
			return;

		graphics::VisualSphere sphere(now.contactPoint, 
			core::QUAT_IDENTITY, radius, graphics::VisualEntity::Style::SOLID_COLOR, 
			graphics::COLOR_CONTACT, 0.5f);

		sphere.render(renderer);

		camera.renderLabel(now.contactPoint, true, "PathSimContactPointLabel", 
			fmt::sprintf("(%.3f, %.3f, %.3f)", now.contactPoint.x, now.contactPoint.y, now.contactPoint.z), 
			graphics::COLOR_CONTACT);
	}

	float Simulation::Timeline::timeOf(int timestepIndex) const
	{
		return timestepIndex * core::TIME_STEP;
	}
	float Simulation::Timeline::backTime() const
	{
		return timeOf(size() - 1);
	}

	Simulation::Timestep::Timestep(
		const glm::vec3& position, const glm::quat& orientation, 
		const glm::vec3& velocity, const glm::vec3& angVelocity,
		const glm::vec3& contactPoint)
		: position(position), orientation(orientation), velocity(velocity), 
		angVelocity(angVelocity), contactPoint(contactPoint)
	{
	}

}