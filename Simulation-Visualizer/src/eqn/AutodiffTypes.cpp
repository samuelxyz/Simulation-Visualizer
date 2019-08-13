#include "core/stdafx.h"

glm::vec3 eqn::glm_cast(const Eigen::Vector3dual & v)
{
	return glm::vec3(
		static_cast<float>(static_cast<double>(v(0))),
		static_cast<float>(static_cast<double>(v(1))),
		static_cast<float>(static_cast<double>(v(2)))
	);
}

glm::quat eqn::glm_cast(const Eigen::Vector4dual & qu)
{
	return glm::quat(
		static_cast<float>(static_cast<double>(qu(0))),
		static_cast<float>(static_cast<double>(qu(1))),
		static_cast<float>(static_cast<double>(qu(2))),
		static_cast<float>(static_cast<double>(qu(3)))
	);
}

Eigen::Vector3dual eqn::dual_cast(const glm::vec3 & v)
{
	return Eigen::Vector3dual(v.x, v.y, v.z);
}

Eigen::Vector4dual eqn::dual_cast(const glm::quat & qu)
{
	return Eigen::Vector4dual(qu.w, qu.x, qu.y, qu.z);
}
