extern "C" {
#include <path_standalone/Standalone_Path.h>
}
#include <iostream>

int pathTest()
{
	int n_dimensions = 2;
	int n_nonzeros = 4;
	int status;

	double z[2];
	double F_on_z[2];
	double lower_bounds[2]{ 0, 20 };
	double upper_bounds[2]{ 1e20, 1e20 };

	pathMain(n_dimensions, n_nonzeros, &status, z, F_on_z, lower_bounds, upper_bounds);

	std::cout << std::endl << "--- Sam Tan's output ---" << std::endl;
	std::cout << "Status: " << status << std::endl;
	std::cout << "z: (" << z[0] << ", " << z[1] << ")" << std::endl;
	std::cout << "F: (" << F_on_z[0] << ", " << F_on_z[1] << ")" << std::endl;

	return 0;
}

int funcEval(int n, double* z, double* F)
{
	// F = Mz + q, M = ((2 1   and q = ((-5)
	//                  (1 2))          (-6))
	F[0] = (2 * z[0] + 1 * z[1]) + -5;
	F[1] = (1 * z[0] + 2 * z[1]) + -6;

	return 0;
}

int jacEval(int n, int nnz, double* z, int* indices_of_used_columns,
	int* len_of_each_column, int* row_index_of_each_value, double* values)
{
	// jacobian = the M matrix from funcEval

	// fill in the structure
	// jacobian indices start from 1 but arrays are from 0 - keep it straight lol
	indices_of_used_columns[0] = 1;
	indices_of_used_columns[1] = 2;
	len_of_each_column[0] = 2;
	len_of_each_column[1] = 2;
	row_index_of_each_value[0] = 1; row_index_of_each_value[1] = 2;
	row_index_of_each_value[2] = 1; row_index_of_each_value[3] = 2;

	// fill in jacobian values
	values[0] = 2; values[1] = 1;
	values[2] = 1; values[3] = 2;

	return 0;
}