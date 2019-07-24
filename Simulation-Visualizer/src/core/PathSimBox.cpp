#include "core/stdafx.h"
#include "PathSimBox.h"
#include "entity/Box.h"

namespace core {

	PathSimBox::PathSimBox()
		: PathSim()
	{
		z = new double[n()];
		F = new double[n()];
		lower_bounds = new double[n()];
		upper_bounds = new double[n()];

		for (int i = 0; i < n(); ++i)
		{
			if (i <= 14)
				lower_bounds[i] = -PATH_INFINITY;
			else
				lower_bounds[i] = 0.0;
			upper_bounds[i] = PATH_INFINITY;
			z[i] = 0.0;
		}
	}

	PathSimBox::~PathSimBox()
	{
	}

	void PathSimBox::setTarget(entity::Entity* e)
	{
		PathSim::setTarget(e);
		
		entity::Box* b = dynamic_cast<entity::Box*>(e);
		assert(b != nullptr && "PathSimBox's target must be a Box");

		len = b->getSizeX();
		wid = b->getSizeY();
		heg = b->getSizeZ();
	}

	int PathSimBox::funcEval(double* z, double* F)
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

		double q0 = -(2.0*((h*q1_o*w_x)/2.0 - q0_o + (h*q2_o*w_y)/2.0 + (h*q3_o*w_z)/2.0))/std::sqrt(h_sq*w_x*w_x + h_sq*w_y*w_y + h_sq*w_z*w_z + 4.0);
		double q1 = (2.0*(q1_o + (h*q0_o*w_x)/2.0 + (h*q3_o*w_y)/2.0 - (h*q2_o*w_z)/2.0))/std::sqrt(h_sq*w_x*w_x + h_sq*w_y*w_y + h_sq*w_z*w_z + 4.0);
		double q2 = (2.0*(q2_o - (h*q3_o*w_x)/2.0 + (h*q0_o*w_y)/2.0 + (h*q1_o*w_z)/2.0))/std::sqrt(h_sq*w_x*w_x + h_sq*w_y*w_y + h_sq*w_z*w_z + 4.0);
		double q3 = (2.0*(q3_o + (h*q2_o*w_x)/2.0 - (h*q1_o*w_y)/2.0 + (h*q0_o*w_z)/2.0))/std::sqrt(h_sq*w_x*w_x + h_sq*w_y*w_y + h_sq*w_z*w_z + 4.0);

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

	int PathSimBox::jacEval(double* z, int* column_starting_indices,
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

		double q0 = -(2.0*((h*q1_o*w_x)/2.0 - q0_o + (h*q2_o*w_y)/2.0 + (h*q3_o*w_z)/2.0))/std::sqrt(h_sq*w_x*w_x + h_sq*w_y*w_y + h_sq*w_z*w_z + 4.0);
		double q1 = (2.0*(q1_o + (h*q0_o*w_x)/2.0 + (h*q3_o*w_y)/2.0 - (h*q2_o*w_z)/2.0))/std::sqrt(h_sq*w_x*w_x + h_sq*w_y*w_y + h_sq*w_z*w_z + 4.0);
		double q2 = (2.0*(q2_o - (h*q3_o*w_x)/2.0 + (h*q0_o*w_y)/2.0 + (h*q1_o*w_z)/2.0))/std::sqrt(h_sq*w_x*w_x + h_sq*w_y*w_y + h_sq*w_z*w_z + 4.0);
		double q3 = (2.0*(q3_o + (h*q2_o*w_x)/2.0 - (h*q1_o*w_y)/2.0 + (h*q0_o*w_z)/2.0))/std::sqrt(h_sq*w_x*w_x + h_sq*w_y*w_y + h_sq*w_z*w_z + 4.0);

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
			4*I_xx*R11*q0 - 4*I_yy*R12*q3 + 4*I_zz*R13*q2, 4*I_xx*R11*q1 + 4*I_yy*R12*q2 + 4*I_zz*R13*q3, 4*I_yy*R12*q1 - 4*I_xx*R11*q2 + 4*I_zz*R13*q0, 4*I_zz*R13*q1 - 4*I_yy*R12*q0 - 4*I_xx*R11*q3,
			2*I_xx*R11*q3 + 2*I_xx*R21*q0 + 2*I_yy*R12*q0 - 2*I_yy*R22*q3 - 2*I_zz*R13*q1 + 2*I_zz*R23*q2, 2*I_xx*R11*q2 + 2*I_xx*R21*q1 - 2*I_yy*R12*q1 + 2*I_yy*R22*q2 - 2*I_zz*R13*q0 + 2*I_zz*R23*q3, 2*I_xx*R11*q1 - 2*I_xx*R21*q2 + 2*I_yy*R12*q2 + 2*I_yy*R22*q1 + 2*I_zz*R13*q3 + 2*I_zz*R23*q0, 2*I_xx*R11*q0 - 2*I_xx*R21*q3 - 2*I_yy*R12*q3 - 2*I_yy*R22*q0 + 2*I_zz*R13*q2 + 2*I_zz*R23*q1,
			2*I_xx*R31*q0 - 2*I_xx*R11*q2 + 2*I_yy*R12*q1 - 2*I_yy*R32*q3 + 2*I_zz*R13*q0 + 2*I_zz*R33*q2, 2*I_xx*R11*q3 + 2*I_xx*R31*q1 + 2*I_yy*R12*q0 + 2*I_yy*R32*q2 - 2*I_zz*R13*q1 + 2*I_zz*R33*q3, 2*I_yy*R12*q3 - 2*I_xx*R31*q2 - 2*I_xx*R11*q0 + 2*I_yy*R32*q1 - 2*I_zz*R13*q2 + 2*I_zz*R33*q0, 2*I_xx*R11*q1 - 2*I_xx*R31*q3 + 2*I_yy*R12*q2 - 2*I_yy*R32*q0 + 2*I_zz*R13*q3 + 2*I_zz*R33*q1,
			2*I_xx*R11*q3 + 2*I_xx*R21*q0 + 2*I_yy*R12*q0 - 2*I_yy*R22*q3 - 2*I_zz*R13*q1 + 2*I_zz*R23*q2, 2*I_xx*R11*q2 + 2*I_xx*R21*q1 - 2*I_yy*R12*q1 + 2*I_yy*R22*q2 - 2*I_zz*R13*q0 + 2*I_zz*R23*q3, 2*I_xx*R11*q1 - 2*I_xx*R21*q2 + 2*I_yy*R12*q2 + 2*I_yy*R22*q1 + 2*I_zz*R13*q3 + 2*I_zz*R23*q0, 2*I_xx*R11*q0 - 2*I_xx*R21*q3 - 2*I_yy*R12*q3 - 2*I_yy*R22*q0 + 2*I_zz*R13*q2 + 2*I_zz*R23*q1,
			4*I_xx*R21*q3 + 4*I_yy*R22*q0 - 4*I_zz*R23*q1, 4*I_xx*R21*q2 - 4*I_yy*R22*q1 - 4*I_zz*R23*q0, 4*I_xx*R21*q1 + 4*I_yy*R22*q2 + 4*I_zz*R23*q3, 4*I_xx*R21*q0 - 4*I_yy*R22*q3 + 4*I_zz*R23*q2,
			2*I_xx*R31*q3 - 2*I_xx*R21*q2 + 2*I_yy*R22*q1 + 2*I_yy*R32*q0 + 2*I_zz*R23*q0 - 2*I_zz*R33*q1, 2*I_xx*R21*q3 + 2*I_xx*R31*q2 + 2*I_yy*R22*q0 - 2*I_yy*R32*q1 - 2*I_zz*R23*q1 - 2*I_zz*R33*q0, 2*I_xx*R31*q1 - 2*I_xx*R21*q0 + 2*I_yy*R22*q3 + 2*I_yy*R32*q2 - 2*I_zz*R23*q2 + 2*I_zz*R33*q3, 2*I_xx*R21*q1 + 2*I_xx*R31*q0 + 2*I_yy*R22*q2 - 2*I_yy*R32*q3 + 2*I_zz*R23*q3 + 2*I_zz*R33*q2,
			2*I_xx*R31*q0 - 2*I_xx*R11*q2 + 2*I_yy*R12*q1 - 2*I_yy*R32*q3 + 2*I_zz*R13*q0 + 2*I_zz*R33*q2, 2*I_xx*R11*q3 + 2*I_xx*R31*q1 + 2*I_yy*R12*q0 + 2*I_yy*R32*q2 - 2*I_zz*R13*q1 + 2*I_zz*R33*q3, 2*I_yy*R12*q3 - 2*I_xx*R31*q2 - 2*I_xx*R11*q0 + 2*I_yy*R32*q1 - 2*I_zz*R13*q2 + 2*I_zz*R33*q0, 2*I_xx*R11*q1 - 2*I_xx*R31*q3 + 2*I_yy*R12*q2 - 2*I_yy*R32*q0 + 2*I_zz*R13*q3 + 2*I_zz*R33*q1,
			2*I_xx*R31*q3 - 2*I_xx*R21*q2 + 2*I_yy*R22*q1 + 2*I_yy*R32*q0 + 2*I_zz*R23*q0 - 2*I_zz*R33*q1, 2*I_xx*R21*q3 + 2*I_xx*R31*q2 + 2*I_yy*R22*q0 - 2*I_yy*R32*q1 - 2*I_zz*R23*q1 - 2*I_zz*R33*q0, 2*I_xx*R31*q1 - 2*I_xx*R21*q0 + 2*I_yy*R22*q3 + 2*I_yy*R32*q2 - 2*I_zz*R23*q2 + 2*I_zz*R33*q3, 2*I_xx*R21*q1 + 2*I_xx*R31*q0 + 2*I_yy*R22*q2 - 2*I_yy*R32*q3 + 2*I_zz*R23*q3 + 2*I_zz*R33*q2,
			4*I_yy*R32*q1 - 4*I_xx*R31*q2 + 4*I_zz*R33*q0, 4*I_xx*R31*q3 + 4*I_yy*R32*q0 - 4*I_zz*R33*q1, 4*I_yy*R32*q3 - 4*I_xx*R31*q0 - 4*I_zz*R33*q2, 4*I_xx*R31*q1 + 4*I_yy*R32*q2 + 4*I_zz*R33*q3;

		J2.block<3, 3>(42, 0) = h*Eigen::Matrix3d::Identity();

		Eigen::MatrixXd J3 = Eigen::MatrixXd::Zero(28, 24);
		J3.topRows(24).setIdentity();
		double N_J3 = std::pow(h_sq*w_x*w_x + h_sq*w_y*w_y + h_sq*w_z*w_z + 4, 1.5);
		J3.block<4, 3>(24, 3) <<
			-(h*(q1_o*h_sq*w_y*w_y - q2_o*w_x*h_sq*w_y + q1_o*h_sq*w_z*w_z - q3_o*w_x*h_sq*w_z + 2*q0_o*w_x*h + 4*q1_o))/N_J3, -(h*(q2_o*h_sq*w_x*w_x - q1_o*w_y*h_sq*w_x + q2_o*h_sq*w_z*w_z - q3_o*w_y*h_sq*w_z + 2*q0_o*w_y*h + 4*q2_o))/N_J3, -(h*(q3_o*h_sq*w_x*w_x - q1_o*w_z*h_sq*w_x + q3_o*h_sq*w_y*w_y - q2_o*w_z*h_sq*w_y + 2*q0_o*w_z*h + 4*q3_o))/N_J3,
			(h*(q0_o*h_sq*w_y*w_y - q3_o*w_x*h_sq*w_y + q0_o*h_sq*w_z*w_z + q2_o*w_x*h_sq*w_z - 2*q1_o*w_x*h + 4*q0_o))/N_J3, (h*(q3_o*h_sq*w_x*w_x - q0_o*w_y*h_sq*w_x + q3_o*h_sq*w_z*w_z + q2_o*w_y*h_sq*w_z - 2*q1_o*w_y*h + 4*q3_o))/N_J3, -(h*(q2_o*h_sq*w_x*w_x + q0_o*w_z*h_sq*w_x + q2_o*h_sq*w_y*w_y + q3_o*w_z*h_sq*w_y + 2*q1_o*w_z*h + 4*q2_o))/N_J3,
			-(h*(q3_o*h_sq*w_y*w_y + q0_o*w_x*h_sq*w_y + q3_o*h_sq*w_z*w_z + q1_o*w_x*h_sq*w_z + 2*q2_o*w_x*h + 4*q3_o))/N_J3, (h*(q0_o*h_sq*w_x*w_x + q3_o*w_y*h_sq*w_x + q0_o*h_sq*w_z*w_z - q1_o*w_y*h_sq*w_z - 2*q2_o*w_y*h + 4*q0_o))/N_J3, (h*(q1_o*h_sq*w_x*w_x + q3_o*w_z*h_sq*w_x + q1_o*h_sq*w_y*w_y - q0_o*w_z*h_sq*w_y - 2*q2_o*w_z*h + 4*q1_o))/N_J3,
			(h*(q2_o*h_sq*w_y*w_y + q1_o*w_x*h_sq*w_y + q2_o*h_sq*w_z*w_z - q0_o*w_x*h_sq*w_z - 2*q3_o*w_x*h + 4*q2_o))/N_J3, -(h*(q1_o*h_sq*w_x*w_x + q2_o*w_y*h_sq*w_x + q1_o*h_sq*w_z*w_z + q0_o*w_y*h_sq*w_z + 2*q3_o*w_y*h + 4*q1_o))/N_J3, (h*(q0_o*h_sq*w_x*w_x - q2_o*w_z*h_sq*w_x + q0_o*h_sq*w_y*w_y + q1_o*w_z*h_sq*w_y - 2*q3_o*w_z*h + 4*q0_o))/N_J3;

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

	void PathSimBox::captureTargetState(bool updateGuesses)
	{
		PathSim::captureTargetState(updateGuesses); // update cache

		// variables
		z[0] = v_xo;
		z[1] = v_yo;
		z[2] = v_zo;
		z[3] = w_xo;
		z[4] = w_yo;
		z[5] = w_zo;
		if (updateGuesses)
		{
			// guess contact point by finding lowest point
			glm::vec3 guess = target->guessECP();

			z[6] = guess.x;
			z[7] = guess.y;
			z[8] = guess.z;
			z[9] = guess.x;
			z[10] = guess.y;
			z[11] = graphics::FLOOR_Z;
		}
	}

	void PathSimBox::printZ() const
	{
		std::cout << "---Sam Tan's output---\nStatus: " << status << " (" << statusCodes[status] << ")" <<
			"\n          Z:\n-----------------------";
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

	void PathSimBox::printConstants() const
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
}