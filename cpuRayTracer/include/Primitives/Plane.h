#pragma once
#include "Primitive.h"

namespace CRT
{
	struct Ray;
	struct HitInfo;

	/**
	 * @brief A finite plane primitive, contains a transform for translation, rotation and scale.
	 */
	class Plane : public Primitive
	{
	public:
		Plane() = default;
		~Plane() = default;

		bool Intersect(const Ray& ray, HitInfo& hitInfo, const float maxRayLength) override;
		glm::vec3 GetNormal();
	};
}