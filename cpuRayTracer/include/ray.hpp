#pragma once
#include <vec3.hpp>

namespace CRT
{
	class Primitive;

	/**
	 * @brief A ray represents an infinite line across a direction.
	 * @note Currently also contains inheritable properties, stored in the path of the sample from the camera. This should turn
	 * into it's own path class.
	 */
	struct Ray
	{
		glm::vec3 Origin = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 Direction = glm::vec3(0.0f, 0.0f, 1.0f);

		/* #Todo: When participating mediums like fog are added this can be removed as it's technically "anywhere". This is currently done to normalize the HDRI color
		 * contribution to get rid of fireflies that would otherwise be visible in the image. */
		bool IsDialectricPath = false;
	};

	/**
	 * @brief The hitinfo is used to store hit results, these do not inherit from one another, as they are always the result
	 * of a ray-primitive intersection.
	 */
	struct HitInfo
	{
		Primitive* HitPrimitive = nullptr;
		glm::vec3 Point = glm::vec3(0.0f);
		glm::vec3 Normal = glm::vec3(0.0f, 1.0f, 0.0f);
		float Distance = 0.0f;
#if defined RENDER_BVH_HEATMAP
		int TraversalSteps = 0;
#endif
	};
}