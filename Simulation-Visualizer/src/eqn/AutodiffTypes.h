#pragma once

#include <Eigen/Dense>
#include "autodiff/forward.hpp"
#include "autodiff/forward/eigen.hpp"

namespace Eigen {
	typedef Matrix<autodiff::dual, 6, 1, 0, 6, 1>	Vector6dual;
	typedef Matrix<autodiff::dual, -1, 3, 0, -1, 3>	GeometryGradients;

}
namespace eqn {
	glm::vec3 glm_cast(const Eigen::Vector3dual& v)
	{
		return glm::vec3(
			static_cast<float>(static_cast<double>(v(0))),
			static_cast<float>(static_cast<double>(v(1))),
			static_cast<float>(static_cast<double>(v(2)))
		);
	}

	glm::quat glm_cast(const Eigen::Vector4dual& qu)
	{
		return glm::quat(
			static_cast<float>(static_cast<double>(qu(0))),
			static_cast<float>(static_cast<double>(qu(1))),
			static_cast<float>(static_cast<double>(qu(2))),
			static_cast<float>(static_cast<double>(qu(3)))
		);
	}

	Eigen::Vector3dual dual_cast(const glm::vec3& v)
	{
		return Eigen::Vector3dual(v.x, v.y, v.z);
	}

	Eigen::Vector4dual dual_cast(const glm::quat& qu)
	{
		return Eigen::Vector4dual(qu.w, qu.x, qu.y, qu.z);
	}
}