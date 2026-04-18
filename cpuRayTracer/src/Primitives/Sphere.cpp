#include "primitives/sphere.hpp"
#include <geometric.hpp>
#include "ray.hpp"

namespace CRT
{
	bool Sphere::Intersect(const Ray& ray, HitInfo& hitInfo, const float maxRayLength)
	{
		//Transform the ray to object space.
		const glm::mat4 inverse = glm::inverse(m_transform.GetMatrix());
		const Ray localSpaceRay =
		{
			inverse * glm::vec4(ray.Origin, 1.0f),
			inverse * glm::vec4(ray.Direction, 0.0f)
		};

		//Retrieve the abc values for the quadratic equation.
		const glm::vec3 rayFromCenter = localSpaceRay.Origin;
		const float a = glm::dot(localSpaceRay.Direction, localSpaceRay.Direction);
		const float b = 2 * glm::dot(localSpaceRay.Direction, rayFromCenter);
		const float c = glm::dot(rayFromCenter, rayFromCenter) - 1.0f;

		//Check discriminant of the quadratic for potentional hits.
		const float discriminant = b * b - 4 * a * c;
		if (discriminant < 0) return false;

		//Solve the quadratic equation.
		float t0, t1;
		if (discriminant == 0)
		{
			t0 = -0.5f * b / a;
			t1 = -0.5f * b / a;
		}
		else
		{
			const float q = (b > 0)
				? -0.5f * (b + sqrtf(discriminant))
				: -0.5f * (b - sqrtf(discriminant));
			t0 = q / a;
			t1 = c / q;
		}

		if (t0 > t1)
		{
			std::swap(t0, t1);
		}

		if (t0 < 0.0f)
		{
			t0 = t1;
			if (t0 < 0.0f)
				return false;
		}

		if (t0 < maxRayLength)
		{
			//Calculate the hit point and normal in worldspace.
			hitInfo.HitPrimitive = this;
			hitInfo.Distance = t0;

			hitInfo.Point = ray.Origin + ray.Direction * t0;
			hitInfo.Normal = glm::transpose(inverse) * glm::vec4(GetNormal(hitInfo.Point), 0.0f);
			hitInfo.Normal = glm::normalize(hitInfo.Normal);
			return true;
		}
		return false;
	}

	glm::vec3 Sphere::GetNormal(const glm::vec3& point)
	{
		glm::vec3 normal = glm::inverse(m_transform.GetMatrix()) * glm::vec4(point, 1.0f);
		return glm::normalize(normal);
	}
}