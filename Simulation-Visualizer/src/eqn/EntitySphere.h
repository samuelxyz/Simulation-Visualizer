#pragma once

#include "EntityE.h"

namespace entity {
	class Sphere;
}

namespace eqn {
	struct EntitySphere : public EntityE
	{
		EntitySphere(entity::Sphere* target);
		virtual ~EntitySphere() = default;

		autodiff::dual radius;

		Eigen::VectorXdual aX_Geometry(Eigen::Vector3dual& v) override;
		Eigen::GeometryGradients aX3_GeometryGradients(Eigen::Vector3dual& v) override;

		virtual int getNumGeometryFuncs() const override { return 1; }
	};
}