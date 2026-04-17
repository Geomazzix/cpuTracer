#pragma once
#include <array>
#include <vec3.hpp>
#include "Primitive.h"

namespace CRT
{
	/**
	 * @brief The box primitive represents 2 corner points in space, which checks if a ray intersects in between.
	 */
	class Box final : public Primitive
	{
	public:
		Box();
		~Box() override = default;

		bool Intersect(const Ray& ray, HitInfo& hit, const float maxRayLength) override;
		glm::vec3 GetNormal(const glm::vec3& point);

	private:
		std::array<glm::vec3, 2> m_bounds;
	};
}