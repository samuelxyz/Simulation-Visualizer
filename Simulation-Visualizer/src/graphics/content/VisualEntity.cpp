#include "core/stdafx.h"
#include "VisualEntity.h"

namespace graphics {

	VisualEntity::VisualEntity(glm::vec3 position, glm::quat orientation, 
		Style style, glm::vec4 color, float shadeFactor)
		: position(position), orientation(orientation), 
		style(style), color(color), shadeFactor(shadeFactor)
	{
	}

	VisualEntity::~VisualEntity()
	{
	}

	glm::vec3 VisualEntity::toLocalFrame(glm::vec3 worldVec, bool useCachedOrientation) const
	{
		// inverse == transpose for rotation matrices
		static glm::mat3 matrix = glm::transpose(glm::toMat3(orientation));
		if (!useCachedOrientation)
			matrix = glm::transpose(glm::toMat3(orientation));

		return matrix * (worldVec - position);
	}

	glm::vec3 VisualEntity::toWorldFrame(glm::vec3 localVec, bool useCachedOrientation) const
	{
		static glm::mat3 matrix = glm::toMat3(orientation);
		if (!useCachedOrientation)
			matrix = glm::toMat3(orientation);

		return matrix * localVec + position;
	}

}