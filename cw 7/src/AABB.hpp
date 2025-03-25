#ifndef AABB_HPP
#define AABB_HPP

#include <glm.hpp> 

struct AABB {
	glm::vec3 min;
	glm::vec3 max;
};


AABB transformAABB(glm::vec3 min, glm::vec3 max, const glm::mat4& modelMatrix)
{
	// Transformowanie punktów min i max przez macierz modelu
	glm::vec4 transformedMin = modelMatrix * glm::vec4(min, 1.0f);
	glm::vec4 transformedMax = modelMatrix * glm::vec4(max, 1.0f);

	// Zwracanie przekszta³conych punktów w strukturze
	AABB transformedAABB;
	transformedAABB.min = glm::vec3(transformedMin.x, transformedMin.y, transformedMin.z);
	transformedAABB.max = glm::vec3(transformedMax.x, transformedMax.y, transformedMax.z);

	return transformedAABB;
}

bool checkAABBCollision(const AABB& a, const AABB& b) {
	return (a.max.x > b.min.x && a.min.x < b.max.x) &&
		(a.max.y > b.min.y && a.min.y < b.max.y) &&
		(a.max.z > b.min.z && a.min.z < b.max.z);
}

#endif 