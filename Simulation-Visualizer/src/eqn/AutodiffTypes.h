#pragma once

#include <Eigen/Dense>
#include "autodiff/forward.hpp"
#include "autodiff/forward/eigen.hpp"

namespace Eigen {
	typedef Matrix<autodiff::dual, 6, 1, 0, 6, 1>	Vector6dual;
	typedef Matrix<autodiff::dual, -1, 3, 0, -1, 3>	GeometryGradients;
}