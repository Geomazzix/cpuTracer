#pragma once
#include <algorithm>
#include <vec3.hpp>

namespace CRT
{
	static glm::vec3 Min(const glm::vec3& a, const glm::vec3& b)
	{
		return glm::vec3(
			std::min(a.x, b.x),
			std::min(a.y, b.y),
			std::min(a.z, b.z)
		);
	}

	static glm::vec3 Max(const glm::vec3& a, const glm::vec3& b)
	{
		return glm::vec3(
			std::max(a.x, b.x),
			std::max(a.y, b.y),
			std::max(a.z, b.z)
		);
	}
}