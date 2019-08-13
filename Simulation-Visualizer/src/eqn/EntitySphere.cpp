#include "core/stdafx.h"
#include "EntitySphere.h"
#include "entity/Sphere.h"

namespace eqn
{
	EntitySphere::EntitySphere(const entity::Sphere* target)
		: EntityE(target), radius(target->getRadius())
	{
	}

	Eigen::VectorXdual EntitySphere::aX_Geometry(Eigen::Vector3dual& v)
	{
		Eigen::VectorXdual result(1);
		result << (v-q).dot(v-q) - radius*radius;
		return result;
	}

	Eigen::GeometryGradients EntitySphere::aX3_GeometryGradients(Eigen::Vector3dual& v)
	{
		return 2*(v-q).transpose();
	}
}
