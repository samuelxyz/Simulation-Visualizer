#include "core/stdafx.h"
#include "VisualEntity.h"

namespace graphics {

	VisualEntity::VisualEntity(glm::vec3 position, glm::quat orientation, 
		Style style, glm::vec4 color)
		: position(position), orientation(orientation), style(style), color(color)
	{
	}

	VisualEntity::~VisualEntity()
	{
	}

	glm::vec3 VisualEntity::toLocalFrame(glm::vec3 worldVec) const
	{
		//return glm::inverse(glm::toMat3(orientation)) * (worldVec - position);
		return glm::transpose(glm::toMat3(orientation)) * (worldVec - position);
		// inverse == transpose for rotation matrices
	}

	glm::vec3 VisualEntity::toWorldFrame(glm::vec3 localVec) const
	{
		return glm::toMat3(orientation) * localVec + position;
	}

}