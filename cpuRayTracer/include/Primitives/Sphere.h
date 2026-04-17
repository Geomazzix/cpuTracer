#pragma once
#include <vec3.hpp>
#include "Primitive.h"

namespace CRT
{
	struct Ray;

	/**
	 * @brief A transformable sphere (meaning can be ellipsoid).
	 * @note Uses a transform to allow scaling, meaning that the intersection is a regular sphere intersection.
	 */
	class Sphere : public Primitive
	{
	public:
		Sphere() = default;
		~Sphere() = default;

		bool Intersect(const Ray& ray, HitInfo& hitInfo, const float maxRayLength) override;
		glm::vec3 GetNormal(const glm::vec3& point);
	};
}