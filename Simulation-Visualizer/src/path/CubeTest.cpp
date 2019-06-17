#include "core/Definitions.h"
#ifdef DO_CUBE_TEST

#include "core/stdafx.h"
extern "C" {
#include <path_standalone/Standalone_Path.h>
}

//function[F, J, domerr] = mcp_funjac_single_convex_contact_patch_cuboid(z, jacflag)

// for brevity
#define EXTRACT_VARS_FROM_Z \
double& v_x = z[0];			\
double& v_y = z[1];			\
double& v_z = z[2];			\
double& w_x = z[3];			\
double& w_y = z[4];			\
double& w_z = z[5];			\
							\
double& a1_x = z[6];		\
double& a1_y = z[7];		\
double& a1_z = z[8];		\
							\
double& a2_x = z[9];		\
double& a2_y = z[10];		\
double& a2_z = z[11];		\
							\
double& p_t = z[12];		\
double& p_o = z[13];		\
double& p_r = z[14];		\
							\
double& sig = z[15];		\
							\
double& l1 = z[16];			\
double& l2 = z[17];			\
double& l3 = z[18];			\
double& l4 = z[19];			\
double& l5 = z[20];			\
double& l6 = z[21];			\
double& l7 = z[22];			\
							\
double& p_n = z[23];		\

// intermediate variables appear in the chain rule
		// z - vector of unknown variables

		// The intermediate variables are :
		// 1.[qo(z) q1(z) q2(z) q3(z)] - Quaternions, which depends on z
		// 2. Rij(qo, q1, q2, q3) - elements in rotation matrix, which depends on the quaternion
		// 3. Iij(qo, q1, q2, q3) - elements in RIR', which depends on the quaternion
		// 3. q_x(z), q_y(z), q_z(z) - position vector in l+1, which depends on q_old and z

		// Thus system of equations F can be described by the unknown variables z or
		// with intermediate variables :
		// 1. F(z, Rij, Iij, q_x, q_y, q_z) - F descriabed by z with intermediate variables
		// 2. F(z) - F described by unknown variables z

		// Based on the chain rule, Jocobain J = J1*J2*J3 where :
		// J - dF(z)/dz Jacobian matrix
		// J1 = dF(z, Rij, Iij, q_x, q_y, q_z)/d[z, Rij, q_x, q_y, q_z]
		// J2 = d[z, Rij(qo, q1, q2, q3), Iij(qo, q1, q2, q3), q_x(z), q_y(z), q_z(z)]/d[z, qo, q1, q2, q3]
		// J3 = d[z, qo(z), q1(z), q2(z), q3(z)]/dz

		//// intermediate variables
#define CALCULATE_INTERMEDIATE_VARS					\
double q0 = -(2.0*((h*q1_o*w_x)/2.0 - q0_o + (h*q2_o*w_y)/2.0 + (h*q3_o*w_z)/2.0))/std::sqrt(h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4.0);\
double q1 = (2.0*(q1_o + (h*q0_o*w_x)/2.0 + (h*q3_o*w_y)/2.0 - (h*q2_o*w_z)/2.0))/std::sqrt(h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4.0);	\
double q2 = (2.0*(q2_o - (h*q3_o*w_x)/2.0 + (h*q0_o*w_y)/2.0 + (h*q1_o*w_z)/2.0))/std::sqrt(h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4.0);	\
double q3 = (2.0*(q3_o + (h*q2_o*w_x)/2.0 - (h*q1_o*w_y)/2.0 + (h*q0_o*w_z)/2.0))/std::sqrt(h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4.0);	\
															\
double q_x = q_xo+h*v_x;									\
double q_y = q_yo+h*v_y;									\
double q_z = q_zo+h*v_z;									\
															\
double R11 = q0*q0 + q1*q1 - q2*q2 - q3*q3;					\
double R12 = 2*q1*q2 - 2*q0*q3;								\
double R13 = 2*q0*q2 + 2*q1*q3;								\
double R21 = 2*q0*q3 + 2*q1*q2;								\
double R22 = q0*q0 - q1*q1 + q2*q2 - q3*q3;					\
double R23 = 2*q2*q3 - 2*q0*q1;								\
double R31 = 2*q1*q3 - 2*q0*q2;								\
double R32 = 2*q0*q1 + 2*q2*q3;								\
double R33 = q0*q0 - q1*q1 - q2*q2 + q3*q3;					\
															\
double I11 = I_xx*R11*R11 + I_yy*R12*R12 + I_zz*R13*R13;	\
double I12 = I_xx*R21*R11 + I_yy* R22*R12 + I_zz*R23*R13;	\
double I13 = I_xx*R31*R11 + I_yy* R32*R12 + I_zz*R33*R13;	\
double I21 = I12;											\
double I22 = I_xx*R21*R21 + I_yy*R22*R22 + I_zz*R23*R23;	\
double I23 = I_xx*R31*R21 + I_yy*R32*R22 + I_zz*R33*R23;	\
double I31 = I13;											\
double I32 = I23;											\
double I33 = I_xx*R31*R31 + I_yy*R32*R32 + I_zz*R33*R33;	\

//#define h core::TIME_STEP
static constexpr double h = static_cast<double>(core::TIME_STEP);
static constexpr double PATH_INFINITY = 1e20;

// cache
//static double q_old[7];
//static double nu_old[6];

static double q_xo;
static double q_yo;
static double q_zo;
static double q0_o;
static double q1_o;
static double q2_o;
static double q3_o;
static double v_xo;
static double v_yo;
static double v_zo;
static double w_xo;
static double w_yo;
static double w_zo;

// constants
//static double m, mu, I_xx, I_yy, I_zz, e_o, e_r, e_t;
static double m = 1.0;
static double mu = 0.5;
static double I_xx = 1.0;
static double I_yy = 1.0;
static double I_zz = 1.0;
static double e_o = 0.5;
static double e_r = 0.5;
static double e_t = 0.5;
static double g = static_cast<double>(core::GRAVITY);
//static double len, wid, heg;
static double len = 1.0;
static double wid = 1.0;
static double heg = 1.0;

// applied
static double p_x, p_y, p_z, p_xt, p_yt, p_zt;

// system state

// function declarations
int funcEval(int n, double* z, double* F);
int jacEval(int n, int nnz, double* z, int* column_starting_indices,
	int* len_of_each_column, int* row_index_of_each_value, double* values);

int cubeTest()
{
	//// load values from previous iteration
	//q_xo = q_old[0];
	//q_yo = q_old[1];
	//q_zo = q_old[2];
	//q0_o = q_old[3];
	//q1_o = q_old[4];
	//q2_o = q_old[5];
	//q3_o = q_old[6];

	//v_xo = nu_old[0];
	//v_yo = nu_old[1];
	//v_zo = nu_old[2];
	//w_xo = nu_old[3];
	//w_yo = nu_old[4];
	//w_zo = nu_old[5];
	
	constexpr int n = 24; // number of unknowns
	constexpr int nnz = 24*24; // overestimate of jacobian nonzeros
	int status;

	double z[n];
	double F[n];
	double lower_bounds[n];
	double upper_bounds[n];
	for (int i = 0; i < n; ++i)
	{
		if (i <= 14)
			lower_bounds[i] = -PATH_INFINITY;
		else
			lower_bounds[i] = 0.0;
		upper_bounds[i] = PATH_INFINITY;
		z[i] = 0.0;
	}

	z[2] = 1.0;

	q_xo = 0.0;
	q_yo = 0.0;
	q_zo = 0.501;
	q0_o = core::QUAT_ZERO.w;
	q1_o = core::QUAT_ZERO.x;
	q2_o = core::QUAT_ZERO.y;
	q3_o = core::QUAT_ZERO.z;
	v_xo = 1.0;
	v_yo = 1.0;
	v_zo = 0.0;
	w_xo = 0.0;
	w_yo = 0.0;
	w_zo = 0.0;

	pathMain(n, nnz, &status, z, F, lower_bounds, upper_bounds);

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

	return 0;
}


int funcEval(int n, double* z, double* F)
{
	// unknown variables

	EXTRACT_VARS_FROM_Z
	CALCULATE_INTERMEDIATE_VARS

	//int domerr = 0;
	//// Newton_Euler equations
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
	F[15] = mu*mu*p_n*p_n - p_r*p_r/e_r*e_r - p_t*p_t/e_t*e_t - p_o*p_o/e_o*e_o;

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
// column_starting_indices is actually called column_start
int jacEval(int n, int nnz, double* z, int* column_starting_indices,
	int* len_of_each_column, int* row_index_of_each_value, double* values)
{
	// unknown variables

	EXTRACT_VARS_FROM_Z
	CALCULATE_INTERMEDIATE_VARS

	// Jacobian matrix(Using chain rules)

	// format: A(row, column)

		// Newton_Euler
	//J1 = zeros(24, 45); // 24 x 45
	Eigen::MatrixXd J1 = Eigen::MatrixXd::Zero(24, 45);
	//J1(1:3, 1 : 3) = -m*eye(3);  //(v_x v_y v_z)
	J1.block<3, 3>(0, 0) = -m*Eigen::Matrix3d::Identity();
	//J1(1:3, [13, 14, 24]) = eye(3); //(p_t, p_o, p_n)
	J1.block<2, 2>(0, 12).setIdentity();
	J1(2, 23) = 1.0;
	//J1(4:6, 4 : 6) = -[I11, I12, I13; I21, I22, I23; I31, I32, I33]; //(w_x w_y w_z)
	J1.block<3, 3>(3, 3) << -I11, -I12, -I13, -I21, -I22, -I23, -I31, -I32, -I33;
	//J1(4:6, 7 : 9) = -product_skew_mat([p_t, p_o, p_n]); //(a1_x a1_y a1_z)
	J1.block<3, 3>(3, 6) << 0.0, p_n, -p_o, -p_n, 0.0, p_t, p_o, -p_t, 0.0;
	//J1(4:6, [13, 14, 24]) = product_skew_mat([a1_x - q_x, a1_y - q_y, a1_z - q_z]); //(p_t, p_o, p_n)
	J1.block<3, 2>(3, 13) << 0.0, q_z-a1_z, a1_z-q_z, 0.0, q_y-a1_y, a1_x-q_x;
	J1.col(23).segment(3, 2) << a1_y-q_y, q_x-a1_x;
	//J1(4:6, 15) = [0; 0; 1]; //p_r
	J1(3, 14) = 1.0;
	//J1(4:6, 34 : 42) = [w_xo - w_x, w_yo - w_y, w_zo - w_z, h*w_xo*w_zo, h*w_yo*w_zo, h*w_zo*w_zo, -h*w_xo*w_yo, -h*w_yo*w_yo, -h*w_yo*w_zo   //(I11-I33)
	//	-h*w_xo*w_zo, -h*w_yo*w_zo, -h*w_zo*w_zo, w_xo - w_x, w_yo - w_y, w_zo - w_z, h*w_xo*w_xo, h*w_xo*w_yo, h*w_xo*w_zo
	//	h*w_xo*w_yo, h*w_yo*w_yo, h*w_yo*w_zo, -h*w_xo*w_xo, -h*w_xo*w_yo, -h*w_xo*w_zo, w_xo - w_x, w_yo - w_y, w_zo - w_z];
	J1.block<3, 9>(3, 33) << w_xo - w_x, w_yo - w_y, w_zo - w_z, h*w_xo*w_zo, h*w_yo*w_zo, h*w_zo*w_zo, -h*w_xo*w_yo, -h*w_yo*w_yo, -h*w_yo*w_zo,
		-h*w_xo*w_zo, -h*w_yo*w_zo, -h*w_zo*w_zo, w_xo - w_x, w_yo - w_y, w_zo - w_z, h*w_xo*w_xo, h*w_xo*w_yo, h*w_xo*w_zo,
		h*w_xo*w_yo, h*w_yo*w_yo, h*w_yo*w_zo, -h*w_xo*w_xo, -h*w_xo*w_yo, -h*w_xo*w_zo, w_xo - w_x, w_yo - w_y, w_zo - w_z;
	//J1(4:6, 43 : 45) = product_skew_mat([p_t, p_o, p_n]); // q_x q_y q_z
	J1.block<3, 3>(3, 42) << 0.0, -p_n, p_o, p_n, 0.0, -p_t, -p_o, p_t, 0.0;
	//	// contact constraints without complementarity equation
	//J1(7:9, 1 : 3) = [e_t*e_t*mu*p_n, 0, 0; 0, e_o*e_o*mu*p_n, 0; 0, 0, 0];  //v_x v_y v_z
	J1(6, 0) = e_t*e_t*mu*p_n;
	J1(7, 1) = e_o*e_o*mu*p_n;
	//J1(7:9, 4 : 6) = [0, e_t*e_t*mu*p_n*(a1_z - q_z), -e_t*e_t*mu*p_n*(a1_y - q_y) //w_x w_y w_z
	//	-e_o*e_o*mu*p_n*(a1_z - q_z), 0, e_o*e_o*mu*p_n*(a1_x - q_x)
	//	0, 0, e_r*e_r*mu*p_n];
	J1.block<3, 3>(6, 3) << 0.0, e_t*e_t*mu*p_n*(a1_z - q_z), -e_t*e_t*mu*p_n*(a1_y - q_y),
		-e_o*e_o*mu*p_n*(a1_z - q_z), 0.0, e_o*e_o*mu*p_n*(a1_x - q_x),
		0.0, 0.0, e_r*e_r*mu*p_n;
	//J1(7:9, 7 : 9) = [0, -e_t*e_t*mu*p_n*w_z, e_t*e_t*mu*p_n*w_y
	//	e_o*e_o*mu*p_n*w_z, 0, -e_o*e_o*mu*p_n*w_x
	//	0, 0, 0];  //a1_x a1_y a1_z
	J1.block<3, 3>(6, 6) << 0.0, -e_t*e_t*mu*p_n*w_z, e_t*e_t*mu*p_n*w_y,
		e_o*e_o*mu*p_n*w_z, 0.0, -e_o*e_o*mu*p_n*w_x,
		0.0, 0.0, 0.0;
	//J1(7:9, 13 : 15) = sig*eye(3); // p_t p_o p_r
	J1.block<3, 3>(6, 12) = sig*Eigen::Matrix3d::Identity();
	//J1(7:9, 16) = [p_t; p_o; p_r]; // sig
	J1.col(15).segment(6, 3) << p_t, p_o, p_r;
	//J1(7:9, 24) = [e_t*e_t*mu*v_x - e_t*e_t*mu*w_z*(a1_y - q_y) + e_t*e_t*mu*w_y*(a1_z - q_z)
	//	e_o*e_o*mu*v_y + e_o*e_o*mu*w_z*(a1_x - q_x) - e_o*e_o*mu*w_x*(a1_z - q_z)
	//	e_r*e_r*mu*w_z]; //p_n
	J1.col(23).segment(6, 3) << e_t*e_t*mu*v_x - e_t*e_t*mu*w_z*(a1_y - q_y) + e_t*e_t*mu*w_y*(a1_z - q_z),
		e_o*e_o*mu*v_y + e_o*e_o*mu*w_z*(a1_x - q_x) - e_o*e_o*mu*w_x*(a1_z - q_z),
		e_r*e_r*mu*w_z;
	//J1(7:9, 43 : 45) = [0, e_t*e_t*mu*p_n*w_z, -e_t*e_t*mu*p_n*w_y
	//	-e_o*e_o*mu*p_n*w_z, 0, e_o*e_o*mu*p_n*w_x
	//	0, 0, 0]; // q_x q_y q_z
	J1.block<3,3>(6, 42) << 0.0, e_t*e_t*mu*p_n*w_z, -e_t*e_t*mu*p_n*w_y, 
			-e_o*e_o*mu*p_n*w_z, 0.0, e_o*e_o*mu*p_n*w_x, 
			0.0, 0.0, 0.0;

	//	// contact constraints without complementarity equation
	//J1(10:12, 7 : 9) = -eye(3);  //a1_x a1_y a1_z
	J1.block<3, 3>(9, 6) = -Eigen::Matrix3d::Identity();
	//J1(10:12, 10 : 12) = eye(3); // a2_x a2_y a2_z
	J1.block<3,3>(9,9).setIdentity();
	//J1(10:12, 23) = [0; 0; 1]; //l7
	J1(11, 23) = 1.0;

	//J1(13:15, 17 : 22) = [-R11, R11, -R12, R12, -R13, R13
	//	-R21, R21, -R22, R22, -R23, R23
	//	-R31, R31, -R32, R32, -R33, R33];  // l1 - l6
	J1.block<3, 6>(12, 16) << -R11, R11, -R12, R12, -R13, R13, 
		-R21, R21, -R22, R22, -R23, R23, 
		-R31, R31, -R32, R32, -R33, R33;
	//J1(13:15, 25 : 33) = [l2 - l1, l4 - l3, l6 - l5, 0, 0, 0, 0, 0, 0
	//	0, 0, 0, l2 - l1, l4 - l3, l6 - l5, 0, 0, 0
	//	0, 0, 0, 0, 0, 0, l2 - l1, l4 - l3, l6 - l5]; //R11 - R33
	J1.row(12).segment(24, 3) << l2 - l1, l4 - l3, l6 - l5;
	J1.row(13).segment(27, 3) << l2 - l1, l4 - l3, l6 - l5;
	J1.row(14).segment(30, 3) << l2 - l1, l4 - l3, l6 - l5;
	//	// complementarity equation in Friction Model
	//J1(16, 13:15) = [-(2*p_t)/e_t*e_t, -(2*p_o)/e_o*e_o, -(2*p_r)/e_r*e_r];  // p_t p_o p_r
	J1.row(15).segment(12, 3) << -(2*p_t)/e_t*e_t, -(2*p_o)/e_o*e_o, -(2*p_r)/e_r*e_r;
	//J1(16, 24) = 2*mu*mu*p_n;
	J1(16, 24) = 2*mu*mu*p_n;

	//// complementarity equations in contact constraints
	//J1(17:22, 7 : 9) = [R11, R21, R31
	//	-R11, -R21, -R31
	//	R12, R22, R32
	//	-R12, -R22, -R32
	//	R13, R23, R33
	//	-R13, -R23, -R33];  //a1_x a1_y a1_z
	J1.block<6, 3>(16, 6) << R11, R21, R31, 
		-R11, -R21, -R31, 
		R12, R22, R32, 
		-R12, -R22, -R32, 
		R13, R23, R33, 
		-R13, -R23, -R33;

	//J1(17:22, 25 : 33) = [a1_x - q_x, 0, 0, a1_y - q_y, 0, 0, a1_z - q_z, 0, 0
	//	q_x - a1_x, 0, 0, q_y - a1_y, 0, 0, q_z - a1_z, 0, 0
	//	0, a1_x - q_x, 0, 0, a1_y - q_y, 0, 0, a1_z - q_z, 0
	//	0, q_x - a1_x, 0, 0, q_y - a1_y, 0, 0, q_z - a1_z, 0
	//	0, 0, a1_x - q_x, 0, 0, a1_y - q_y, 0, 0, a1_z - q_z
	//	0, 0, q_x - a1_x, 0, 0, q_y - a1_y, 0, 0, q_z - a1_z]; //R11 - R33
	J1.block<6, 9>(16, 24) << 
		a1_x - q_x, 0.0, 0.0, a1_y - q_y, 0.0, 0.0, a1_z - q_z, 0.0, 0.0, 
		q_x - a1_x, 0.0, 0.0, q_y - a1_y, 0.0, 0.0, q_z - a1_z, 0.0, 0.0, 
		0.0, a1_x - q_x, 0.0, 0.0, a1_y - q_y, 0.0, 0.0, a1_z - q_z, 0.0, 
		0.0, q_x - a1_x, 0.0, 0.0, q_y - a1_y, 0.0, 0.0, q_z - a1_z, 0.0, 
		0.0, 0.0, a1_x - q_x, 0.0, 0.0, a1_y - q_y, 0.0, 0.0, a1_z - q_z, 
		0.0, 0.0, q_x - a1_x, 0.0, 0.0, q_y - a1_y, 0.0, 0.0, q_z - a1_z;

	//J1(17:22, 43 : 45) = [-R11, -R21, -R31
	//	R11, R21, R31
	//	-R12, -R22, -R32
	//	R12, R22, R32
	//	-R13, -R23, -R33
	//	R13, R23, R33];  //q_x q_y q_z
	J1.block<6, 3>(16, 42) << -R11, -R21, -R31,
		R11, R21, R31,
		-R12, -R22, -R32,
		R12, R22, R32,
		-R13, -R23, -R33,
		R13, R23, R33;
	//J1(23, 12) = -1;
	//J1(24, 9) = 1;
	J1(22, 11) = -1.0;
	J1(23, 8) = 1.0;

	//J2 = zeros(36, 28); // 45 x 28
	Eigen::MatrixXd J2 = Eigen::MatrixXd::Zero(45, 28);
	//J2(1:24, 1 : 24) = eye(24); //dz/dz
	J2.topLeftCorner(24, 24).setIdentity();
	//J2(25:33, end-3 : end) = [2*q0, 2*q1, -2*q2, -2*q3
	//	-2*q3, 2*q2, 2*q1, -2*q0
	//	2*q2, 2*q3, 2*q0, 2*q1
	//	2*q3, 2*q2, 2*q1, 2*q0
	//	2*q0, -2*q1, 2*q2, -2*q3
	//	-2*q1, -2*q0, 2*q3, 2*q2
	//	-2*q2, 2*q3, -2*q0, 2*q1
	//	2*q1, 2*q0, 2*q3, 2*q2
	//	2*q0, -2*q1, -2*q2, 2*q3]; //dRij/d[q0, q1, q2, q3]
	J2.block<9, 4>(24, 24) << 2*q0, 2*q1, -2*q2, -2*q3,
		-2*q3, 2*q2, 2*q1, -2*q0,
		2*q2, 2*q3, 2*q0, 2*q1,
		2*q3, 2*q2, 2*q1, 2*q0,
		2*q0, -2*q1, 2*q2, -2*q3,
		-2*q1, -2*q0, 2*q3, 2*q2,
		-2*q2, 2*q3, -2*q0, 2*q1,
		2*q1, 2*q0, 2*q3, 2*q2,
		2*q0, -2*q1, -2*q2, 2*q3;

	//J2(34:42, end-3 : end) = [4*I_xx*R11*q0 - 4*I_yy*R12*q3 + 4*I_zz*R13*q2, 4*I_xx*R11*q1 + 4*I_yy*R12*q2 + 4*I_zz*R13*q3, 4*I_yy*R12*q1 - 4*I_xx*R11*q2 + 4*I_zz*R13*q0, 4*I_zz*R13*q1 - 4*I_yy*R12*q0 - 4*I_xx*R11*q3
	//	2*I_xx*R11*q3 + 2*I_xx*R21*q0 + 2*I_yy*R12*q0 - 2*I_yy*R22*q3 - 2*I_zz*R13*q1 + 2*I_zz*R23*q2, 2*I_xx*R11*q2 + 2*I_xx*R21*q1 - 2*I_yy*R12*q1 + 2*I_yy*R22*q2 - 2*I_zz*R13*q0 + 2*I_zz*R23*q3, 2*I_xx*R11*q1 - 2*I_xx*R21*q2 + 2*I_yy*R12*q2 + 2*I_yy*R22*q1 + 2*I_zz*R13*q3 + 2*I_zz*R23*q0, 2*I_xx*R11*q0 - 2*I_xx*R21*q3 - 2*I_yy*R12*q3 - 2*I_yy*R22*q0 + 2*I_zz*R13*q2 + 2*I_zz*R23*q1
	//	2*I_xx*R31*q0 - 2*I_xx*R11*q2 + 2*I_yy*R12*q1 - 2*I_yy*R32*q3 + 2*I_zz*R13*q0 + 2*I_zz*R33*q2, 2*I_xx*R11*q3 + 2*I_xx*R31*q1 + 2*I_yy*R12*q0 + 2*I_yy*R32*q2 - 2*I_zz*R13*q1 + 2*I_zz*R33*q3, 2*I_yy*R12*q3 - 2*I_xx*R31*q2 - 2*I_xx*R11*q0 + 2*I_yy*R32*q1 - 2*I_zz*R13*q2 + 2*I_zz*R33*q0, 2*I_xx*R11*q1 - 2*I_xx*R31*q3 + 2*I_yy*R12*q2 - 2*I_yy*R32*q0 + 2*I_zz*R13*q3 + 2*I_zz*R33*q1
	//	2*I_xx*R11*q3 + 2*I_xx*R21*q0 + 2*I_yy*R12*q0 - 2*I_yy*R22*q3 - 2*I_zz*R13*q1 + 2*I_zz*R23*q2, 2*I_xx*R11*q2 + 2*I_xx*R21*q1 - 2*I_yy*R12*q1 + 2*I_yy*R22*q2 - 2*I_zz*R13*q0 + 2*I_zz*R23*q3, 2*I_xx*R11*q1 - 2*I_xx*R21*q2 + 2*I_yy*R12*q2 + 2*I_yy*R22*q1 + 2*I_zz*R13*q3 + 2*I_zz*R23*q0, 2*I_xx*R11*q0 - 2*I_xx*R21*q3 - 2*I_yy*R12*q3 - 2*I_yy*R22*q0 + 2*I_zz*R13*q2 + 2*I_zz*R23*q1
	//	4*I_xx*R21*q3 + 4*I_yy*R22*q0 - 4*I_zz*R23*q1, 4*I_xx*R21*q2 - 4*I_yy*R22*q1 - 4*I_zz*R23*q0, 4*I_xx*R21*q1 + 4*I_yy*R22*q2 + 4*I_zz*R23*q3, 4*I_xx*R21*q0 - 4*I_yy*R22*q3 + 4*I_zz*R23*q2
	//	2*I_xx*R31*q3 - 2*I_xx*R21*q2 + 2*I_yy*R22*q1 + 2*I_yy*R32*q0 + 2*I_zz*R23*q0 - 2*I_zz*R33*q1, 2*I_xx*R21*q3 + 2*I_xx*R31*q2 + 2*I_yy*R22*q0 - 2*I_yy*R32*q1 - 2*I_zz*R23*q1 - 2*I_zz*R33*q0, 2*I_xx*R31*q1 - 2*I_xx*R21*q0 + 2*I_yy*R22*q3 + 2*I_yy*R32*q2 - 2*I_zz*R23*q2 + 2*I_zz*R33*q3, 2*I_xx*R21*q1 + 2*I_xx*R31*q0 + 2*I_yy*R22*q2 - 2*I_yy*R32*q3 + 2*I_zz*R23*q3 + 2*I_zz*R33*q2
	//	2*I_xx*R31*q0 - 2*I_xx*R11*q2 + 2*I_yy*R12*q1 - 2*I_yy*R32*q3 + 2*I_zz*R13*q0 + 2*I_zz*R33*q2, 2*I_xx*R11*q3 + 2*I_xx*R31*q1 + 2*I_yy*R12*q0 + 2*I_yy*R32*q2 - 2*I_zz*R13*q1 + 2*I_zz*R33*q3, 2*I_yy*R12*q3 - 2*I_xx*R31*q2 - 2*I_xx*R11*q0 + 2*I_yy*R32*q1 - 2*I_zz*R13*q2 + 2*I_zz*R33*q0, 2*I_xx*R11*q1 - 2*I_xx*R31*q3 + 2*I_yy*R12*q2 - 2*I_yy*R32*q0 + 2*I_zz*R13*q3 + 2*I_zz*R33*q1
	//	2*I_xx*R31*q3 - 2*I_xx*R21*q2 + 2*I_yy*R22*q1 + 2*I_yy*R32*q0 + 2*I_zz*R23*q0 - 2*I_zz*R33*q1, 2*I_xx*R21*q3 + 2*I_xx*R31*q2 + 2*I_yy*R22*q0 - 2*I_yy*R32*q1 - 2*I_zz*R23*q1 - 2*I_zz*R33*q0, 2*I_xx*R31*q1 - 2*I_xx*R21*q0 + 2*I_yy*R22*q3 + 2*I_yy*R32*q2 - 2*I_zz*R23*q2 + 2*I_zz*R33*q3, 2*I_xx*R21*q1 + 2*I_xx*R31*q0 + 2*I_yy*R22*q2 - 2*I_yy*R32*q3 + 2*I_zz*R23*q3 + 2*I_zz*R33*q2
	//	4*I_yy*R32*q1 - 4*I_xx*R31*q2 + 4*I_zz*R33*q0, 4*I_xx*R31*q3 + 4*I_yy*R32*q0 - 4*I_zz*R33*q1, 4*I_yy*R32*q3 - 4*I_xx*R31*q0 - 4*I_zz*R33*q2, 4*I_xx*R31*q1 + 4*I_yy*R32*q2 + 4*I_zz*R33*q3];
	J2.block<9, 4>(33, 24) << 4*I_xx*R11*q0 - 4*I_yy*R12*q3 + 4*I_zz*R13*q2, 4*I_xx*R11*q1 + 4*I_yy*R12*q2 + 4*I_zz*R13*q3, 4*I_yy*R12*q1 - 4*I_xx*R11*q2 + 4*I_zz*R13*q0, 4*I_zz*R13*q1 - 4*I_yy*R12*q0 - 4*I_xx*R11*q3,
		2*I_xx*R11*q3 + 2*I_xx*R21*q0 + 2*I_yy*R12*q0 - 2*I_yy*R22*q3 - 2*I_zz*R13*q1 + 2*I_zz*R23*q2, 2*I_xx*R11*q2 + 2*I_xx*R21*q1 - 2*I_yy*R12*q1 + 2*I_yy*R22*q2 - 2*I_zz*R13*q0 + 2*I_zz*R23*q3, 2*I_xx*R11*q1 - 2*I_xx*R21*q2 + 2*I_yy*R12*q2 + 2*I_yy*R22*q1 + 2*I_zz*R13*q3 + 2*I_zz*R23*q0, 2*I_xx*R11*q0 - 2*I_xx*R21*q3 - 2*I_yy*R12*q3 - 2*I_yy*R22*q0 + 2*I_zz*R13*q2 + 2*I_zz*R23*q1,
		2*I_xx*R31*q0 - 2*I_xx*R11*q2 + 2*I_yy*R12*q1 - 2*I_yy*R32*q3 + 2*I_zz*R13*q0 + 2*I_zz*R33*q2, 2*I_xx*R11*q3 + 2*I_xx*R31*q1 + 2*I_yy*R12*q0 + 2*I_yy*R32*q2 - 2*I_zz*R13*q1 + 2*I_zz*R33*q3, 2*I_yy*R12*q3 - 2*I_xx*R31*q2 - 2*I_xx*R11*q0 + 2*I_yy*R32*q1 - 2*I_zz*R13*q2 + 2*I_zz*R33*q0, 2*I_xx*R11*q1 - 2*I_xx*R31*q3 + 2*I_yy*R12*q2 - 2*I_yy*R32*q0 + 2*I_zz*R13*q3 + 2*I_zz*R33*q1,
		2*I_xx*R11*q3 + 2*I_xx*R21*q0 + 2*I_yy*R12*q0 - 2*I_yy*R22*q3 - 2*I_zz*R13*q1 + 2*I_zz*R23*q2, 2*I_xx*R11*q2 + 2*I_xx*R21*q1 - 2*I_yy*R12*q1 + 2*I_yy*R22*q2 - 2*I_zz*R13*q0 + 2*I_zz*R23*q3, 2*I_xx*R11*q1 - 2*I_xx*R21*q2 + 2*I_yy*R12*q2 + 2*I_yy*R22*q1 + 2*I_zz*R13*q3 + 2*I_zz*R23*q0, 2*I_xx*R11*q0 - 2*I_xx*R21*q3 - 2*I_yy*R12*q3 - 2*I_yy*R22*q0 + 2*I_zz*R13*q2 + 2*I_zz*R23*q1,
		4*I_xx*R21*q3 + 4*I_yy*R22*q0 - 4*I_zz*R23*q1, 4*I_xx*R21*q2 - 4*I_yy*R22*q1 - 4*I_zz*R23*q0, 4*I_xx*R21*q1 + 4*I_yy*R22*q2 + 4*I_zz*R23*q3, 4*I_xx*R21*q0 - 4*I_yy*R22*q3 + 4*I_zz*R23*q2,
		2*I_xx*R31*q3 - 2*I_xx*R21*q2 + 2*I_yy*R22*q1 + 2*I_yy*R32*q0 + 2*I_zz*R23*q0 - 2*I_zz*R33*q1, 2*I_xx*R21*q3 + 2*I_xx*R31*q2 + 2*I_yy*R22*q0 - 2*I_yy*R32*q1 - 2*I_zz*R23*q1 - 2*I_zz*R33*q0, 2*I_xx*R31*q1 - 2*I_xx*R21*q0 + 2*I_yy*R22*q3 + 2*I_yy*R32*q2 - 2*I_zz*R23*q2 + 2*I_zz*R33*q3, 2*I_xx*R21*q1 + 2*I_xx*R31*q0 + 2*I_yy*R22*q2 - 2*I_yy*R32*q3 + 2*I_zz*R23*q3 + 2*I_zz*R33*q2,
		2*I_xx*R31*q0 - 2*I_xx*R11*q2 + 2*I_yy*R12*q1 - 2*I_yy*R32*q3 + 2*I_zz*R13*q0 + 2*I_zz*R33*q2, 2*I_xx*R11*q3 + 2*I_xx*R31*q1 + 2*I_yy*R12*q0 + 2*I_yy*R32*q2 - 2*I_zz*R13*q1 + 2*I_zz*R33*q3, 2*I_yy*R12*q3 - 2*I_xx*R31*q2 - 2*I_xx*R11*q0 + 2*I_yy*R32*q1 - 2*I_zz*R13*q2 + 2*I_zz*R33*q0, 2*I_xx*R11*q1 - 2*I_xx*R31*q3 + 2*I_yy*R12*q2 - 2*I_yy*R32*q0 + 2*I_zz*R13*q3 + 2*I_zz*R33*q1,
		2*I_xx*R31*q3 - 2*I_xx*R21*q2 + 2*I_yy*R22*q1 + 2*I_yy*R32*q0 + 2*I_zz*R23*q0 - 2*I_zz*R33*q1, 2*I_xx*R21*q3 + 2*I_xx*R31*q2 + 2*I_yy*R22*q0 - 2*I_yy*R32*q1 - 2*I_zz*R23*q1 - 2*I_zz*R33*q0, 2*I_xx*R31*q1 - 2*I_xx*R21*q0 + 2*I_yy*R22*q3 + 2*I_yy*R32*q2 - 2*I_zz*R23*q2 + 2*I_zz*R33*q3, 2*I_xx*R21*q1 + 2*I_xx*R31*q0 + 2*I_yy*R22*q2 - 2*I_yy*R32*q3 + 2*I_zz*R23*q3 + 2*I_zz*R33*q2,
		4*I_yy*R32*q1 - 4*I_xx*R31*q2 + 4*I_zz*R33*q0, 4*I_xx*R31*q3 + 4*I_yy*R32*q0 - 4*I_zz*R33*q1, 4*I_yy*R32*q3 - 4*I_xx*R31*q0 - 4*I_zz*R33*q2, 4*I_xx*R31*q1 + 4*I_yy*R32*q2 + 4*I_zz*R33*q3;

	////dIij/d[q0, q1, q2, q3]
	//J2(43:45, 1 : 3) = h*eye(3);
	J2.block<3, 3>(42, 0) = h*Eigen::Matrix3d::Identity();

	//J3 = zeros(28, 24); // 28 x 24
	Eigen::MatrixXd J3 = Eigen::MatrixXd::Zero(28, 24);
	//J3(1:24, 1 : 24) = eye(24);  //dz/dz
	J3.topRows(24).setIdentity();
	//N_J3 = (h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4)^(3/2);
	double N_J3 = std::pow(h*h*w_x*w_x + h*h*w_y*w_y + h*h*w_z*w_z + 4, 1.5);

	//J3(25:28, 4 : 6) = [-(h*(q1_o*h*h*w_y*w_y - q2_o*w_x*h*h*w_y + q1_o*h*h*w_z*w_z - q3_o*w_x*h*h*w_z + 2*q0_o*w_x*h + 4*q1_o))/N_J3, -(h*(q2_o*h*h*w_x*w_x - q1_o*w_y*h*h*w_x + q2_o*h*h*w_z*w_z - q3_o*w_y*h*h*w_z + 2*q0_o*w_y*h + 4*q2_o))/N_J3, -(h*(q3_o*h*h*w_x*w_x - q1_o*w_z*h*h*w_x + q3_o*h*h*w_y*w_y - q2_o*w_z*h*h*w_y + 2*q0_o*w_z*h + 4*q3_o))/N_J3
	//(h*(q0_o*h*h*w_y*w_y - q3_o*w_x*h*h*w_y + q0_o*h*h*w_z*w_z + q2_o*w_x*h*h*w_z - 2*q1_o*w_x*h + 4*q0_o))/N_J3, (h*(q3_o*h*h*w_x*w_x - q0_o*w_y*h*h*w_x + q3_o*h*h*w_z*w_z + q2_o*w_y*h*h*w_z - 2*q1_o*w_y*h + 4*q3_o))/N_J3, -(h*(q2_o*h*h*w_x*w_x + q0_o*w_z*h*h*w_x + q2_o*h*h*w_y*w_y + q3_o*w_z*h*h*w_y + 2*q1_o*w_z*h + 4*q2_o))/N_J3
	//	-(h*(q3_o*h*h*w_y*w_y + q0_o*w_x*h*h*w_y + q3_o*h*h*w_z*w_z + q1_o*w_x*h*h*w_z + 2*q2_o*w_x*h + 4*q3_o))/N_J3, (h*(q0_o*h*h*w_x*w_x + q3_o*w_y*h*h*w_x + q0_o*h*h*w_z*w_z - q1_o*w_y*h*h*w_z - 2*q2_o*w_y*h + 4*q0_o))/N_J3, (h*(q1_o*h*h*w_x*w_x + q3_o*w_z*h*h*w_x + q1_o*h*h*w_y*w_y - q0_o*w_z*h*h*w_y - 2*q2_o*w_z*h + 4*q1_o))/N_J3
	//	(h*(q2_o*h*h*w_y*w_y + q1_o*w_x*h*h*w_y + q2_o*h*h*w_z*w_z - q0_o*w_x*h*h*w_z - 2*q3_o*w_x*h + 4*q2_o))/N_J3, -(h*(q1_o*h*h*w_x*w_x + q2_o*w_y*h*h*w_x + q1_o*h*h*w_z*w_z + q0_o*w_y*h*h*w_z + 2*q3_o*w_y*h + 4*q1_o))/N_J3, (h*(q0_o*h*h*w_x*w_x - q2_o*w_z*h*h*w_x + q0_o*h*h*w_y*w_y + q1_o*w_z*h*h*w_y - 2*q3_o*w_z*h + 4*q0_o))/N_J3]; //d(q0 q1 q2 q3)/d(wx wy wz)
	J3.block<4, 3>(24, 3) << -(h*(q1_o*h*h*w_y*w_y - q2_o*w_x*h*h*w_y + q1_o*h*h*w_z*w_z - q3_o*w_x*h*h*w_z + 2*q0_o*w_x*h + 4*q1_o))/N_J3, -(h*(q2_o*h*h*w_x*w_x - q1_o*w_y*h*h*w_x + q2_o*h*h*w_z*w_z - q3_o*w_y*h*h*w_z + 2*q0_o*w_y*h + 4*q2_o))/N_J3, -(h*(q3_o*h*h*w_x*w_x - q1_o*w_z*h*h*w_x + q3_o*h*h*w_y*w_y - q2_o*w_z*h*h*w_y + 2*q0_o*w_z*h + 4*q3_o))/N_J3,
		(h*(q0_o*h*h*w_y*w_y - q3_o*w_x*h*h*w_y + q0_o*h*h*w_z*w_z + q2_o*w_x*h*h*w_z - 2*q1_o*w_x*h + 4*q0_o))/N_J3, (h*(q3_o*h*h*w_x*w_x - q0_o*w_y*h*h*w_x + q3_o*h*h*w_z*w_z + q2_o*w_y*h*h*w_z - 2*q1_o*w_y*h + 4*q3_o))/N_J3, -(h*(q2_o*h*h*w_x*w_x + q0_o*w_z*h*h*w_x + q2_o*h*h*w_y*w_y + q3_o*w_z*h*h*w_y + 2*q1_o*w_z*h + 4*q2_o))/N_J3,
		-(h*(q3_o*h*h*w_y*w_y + q0_o*w_x*h*h*w_y + q3_o*h*h*w_z*w_z + q1_o*w_x*h*h*w_z + 2*q2_o*w_x*h + 4*q3_o))/N_J3, (h*(q0_o*h*h*w_x*w_x + q3_o*w_y*h*h*w_x + q0_o*h*h*w_z*w_z - q1_o*w_y*h*h*w_z - 2*q2_o*w_y*h + 4*q0_o))/N_J3, (h*(q1_o*h*h*w_x*w_x + q3_o*w_z*h*h*w_x + q1_o*h*h*w_y*w_y - q0_o*w_z*h*h*w_y - 2*q2_o*w_z*h + 4*q1_o))/N_J3,
		(h*(q2_o*h*h*w_y*w_y + q1_o*w_x*h*h*w_y + q2_o*h*h*w_z*w_z - q0_o*w_x*h*h*w_z - 2*q3_o*w_x*h + 4*q2_o))/N_J3, -(h*(q1_o*h*h*w_x*w_x + q2_o*w_y*h*h*w_x + q1_o*h*h*w_z*w_z + q0_o*w_y*h*h*w_z + 2*q3_o*w_y*h + 4*q1_o))/N_J3, (h*(q0_o*h*h*w_x*w_x - q2_o*w_z*h*h*w_x + q0_o*h*h*w_y*w_y + q1_o*w_z*h*h*w_y - 2*q3_o*w_z*h + 4*q0_o))/N_J3;

	//J = J1*J2*J3;
	//J = sparse(J);

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

//#define DEBUG_SPARSEMATRIX_PRINTOUT
#ifdef DEBUG_SPARSEMATRIX_PRINTOUT
	std::cout << "\n" << column_starting_indices << "\t" << len_of_each_column;
	std::cout << "\nCSI\tLOEC"; // this is a comment
	for (int i = 0; i < J.cols(); ++i) // this is a comment
		std::cout << "\n" << column_starting_indices[i] << "\t" << len_of_each_column[i];
	std::cout << "\n" << &(column_starting_indices[J.cols()]) << "\t" << &(len_of_each_column[J.cols()-1]);

	std::cout << "\n\nValue index:\t" << val_index;

	std::cout << "\n" << row_index_of_each_value << "\t" << values;
	std::cout << "\nRow index\tValue"; // this is a comment
	for (int i = 0; i < val_index; ++i)
		std::cout << "\n" << row_index_of_each_value[i] << "\t" << values[i];
	std::cout << "\n" << &(row_index_of_each_value[val_index]) << "\t" << &(values[val_index]);

	std::cout << std::endl; // this is a comment
#endif
	
	return 0;
}

#undef EXTRACT_VARS_FROM_Z
#undef CALCULATE_INTERMEDIATE_VARS

#endif