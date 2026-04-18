#include "primitives/plane.hpp"
#include <geometric.hpp>
#include "ray.hpp"

namespace CRT
{
	bool Plane::Intersect(const Ray& ray, HitInfo& hitInfo, const float maxRayLength)
	{
		//Transform the ray to object space.
		const glm::mat4 inverse = glm::inverse(m_transform.GetMatrix());
		const Ray localSpaceRay =
		{
			inverse * glm::vec4(ray.Origin, 1.0f),
			inverse * glm::vec4(ray.Direction, 0.0f)
		};

		//Check if the local space ray hits the plane.
		float t = glm::dot(-localSpaceRay.Origin, GetNormal()) / glm::dot(localSpaceRay.Direction, GetNormal());
		if(t > 0.0f)
		{
			const glm::vec3 localHitPoint = localSpaceRay.Origin + localSpaceRay.Direction * t;
			const glm::vec3 position = glm::vec3(m_transform.GetTranslation());
			glm::vec3 scale = m_transform.GetScale();
			if (localHitPoint.x > position.x + scale.x || localHitPoint.x < position.x - scale.x ||
				localHitPoint.z > position.z + scale.z || localHitPoint.z < position.z - scale.z)
				return false;

			if(t < maxRayLength)
			{
				//Upon hit, assign the surface hit values.
				hitInfo.HitPrimitive = this;
				hitInfo.Distance = t;

				hitInfo.Point = ray.Origin + ray.Direction * t;
				hitInfo.Normal = glm::transpose(inverse) * glm::vec4(GetNormal(), 0.0f);
				hitInfo.Normal = glm::normalize(hitInfo.Normal);
				return true;
			}
		}

		return false;
	}

	glm::vec3 Plane::GetNormal()
	{
		return glm::vec3(0.0f, 1.0f, 0.0);
	}
}