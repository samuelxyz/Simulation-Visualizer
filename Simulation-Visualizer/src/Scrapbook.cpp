#include "core/stdafx.h"

void writeOut(Eigen::SparseMatrix<float>& mat, int valnum, int innernum, int outernum)
{
	float* val = mat.valuePtr();
	int* inner = mat.innerIndexPtr();
	//int* innernz = mat.innerNonZeroPtr();
	int* outer = mat.outerIndexPtr();

	std::cout << mat << std::endl;

	std::cout << "Values:\t";
	for (int i = 0; i < valnum; ++i)
		std::cout << val[i] << "\t";

	std::cout << "\nInner:\t";
	for (int i = 0; i < innernum; ++i)
		std::cout << inner[i] << "\t";

	std::cout << "\nOuter:\t";
	for (int i = 0; i < outernum; ++i)
		std::cout << outer[i] << "\t";

	std::cout << std::endl;
}

int scrapbook()
{
	std::cout << "Isnormal:" << 
		"\nNaN\t" << std::isnormal(NAN) <<
		"\n-NaN\t" << std::isnormal(-NAN) <<
		"\nInf\t" << std::isnormal(INFINITY) <<
		"\n-Inf\t" << std::isnormal(-INFINITY) <<
		"\n1.0\t" << std::isnormal(1.0) <<
		"\n-1.0\t" << std::isnormal(-1.0) << 
		"\n1e-12\t" << std::isnormal(1e-12) <<std::endl;

	return 0;
}

int scrapbookOld()
{
	Eigen::SparseMatrix<float> mat(5,5);
	mat.insert(3, 4) = 17.0f;
	mat.insert(0, 2) = 9.0f;
	mat.makeCompressed();
	writeOut(mat,3,3,7);

	Eigen::SparseMatrix<float> mat2(5, 5);
	mat2.insert(1, 0) = 22.0f;
	mat2.insert(2, 0) = 7.0f;
	mat2.insert(0, 1) = 3.0f;
	mat2.insert(2, 1) = 5.0f;
	mat2.insert(4, 2) = 14.0f;
	mat2.insert(2, 3) = 1.0f;
	mat2.insert(1, 4) = 17.0f;
	mat2.insert(4, 4) = 8.0f;
	mat2.makeCompressed();
	writeOut(mat2, 9, 9, 7);

	return 0;
}