#pragma once

#include <Eigen/Dense>
#include "autodiff/forward.hpp"
#include "autodiff/forward/eigen.hpp"

namespace Eigen {
	typedef Matrix<autodiff::dual, 6, 1, 0, 6, 1>	Vector6dual;
	typedef Matrix<autodiff::dual, -1, 3, 0, -1, 3>	GeometryGradients;

}
namespace eqn {
	glm::vec3 glm_cast(const Eigen::Vector3dual& v);

	glm::quat glm_cast(const Eigen::Vector4dual& qu);

	Eigen::Vector3dual dual_cast(const glm::vec3& v);

	Eigen::Vector4dual dual_cast(const glm::quat& qu);
}