#include "primitives/box.hpp"
#include "ray.hpp"

namespace CRT
{
	Box::Box() : 
		m_bounds { glm::vec3(-.5f), glm::vec3(.5f) }
	{ }

	bool Box::Intersect(const Ray& ray, HitInfo& hit, const float maxRayLength)
	{
		//Transform the ray to object space.
		const glm::mat4 inverse = glm::inverse(m_transform.GetMatrix());
		const Ray localSpaceRay =
		{
			inverse * glm::vec4(ray.Origin, 1.0f),
			inverse * glm::vec4(ray.Direction, 0.0f)
		};

		//Calculate X-bounds.
		float t0x = (m_bounds[0].x - localSpaceRay.Origin.x) / localSpaceRay.Direction.x;
		float t1x = (m_bounds[1].x - localSpaceRay.Origin.x) / localSpaceRay.Direction.x;
		if (t0x > t1x)
		{
			std::swap<float>(t0x, t1x);
		}

		//Calculate Y-bounds.
		float t0y = (m_bounds[0].y - localSpaceRay.Origin.y) / localSpaceRay.Direction.y;
		float t1y = (m_bounds[1].y - localSpaceRay.Origin.y) / localSpaceRay.Direction.y;
		if (t0y > t1y)
		{
			std::swap<float>(t0y, t1y);
		}

		//Check if the box renders in front of the camera.
		if (t0x > t1y || t0y > t1x)
		{
			return false;
		}

		//Calculate 2-dim points.
		float tMin = t0y > t0x ? t0y : t0x;
		float tMax = t1y < t1x ? t1y : t1x;

		//Calculate Z-bounds.
		float t0z = (m_bounds[0].z - localSpaceRay.Origin.z) / localSpaceRay.Direction.z;
		float t1z = (m_bounds[1].z - localSpaceRay.Origin.z) / localSpaceRay.Direction.z;
		if (t0z > t1z)
		{
			std::swap<float>(t0z, t1z);
		}

		//Check if the box renders in front of the camera.
		if (tMin > t1z || t0z > tMax)
		{
			return false;
		}

		if (t0z > tMin)
		{
			tMin = t0z;
		}

		if (t1z < tMax)
		{
			tMax = t1z;
		}

		if (tMin < 0.0f)
		{
			tMin = tMax;
		}

		if (tMax <= 0.0f || tMin > maxRayLength)
		{
			return false;
		}

		if(tMin < maxRayLength)
		{
			//Calculate the hit point and normal in worldspace.
			hit.HitPrimitive = this;
			hit.Distance = tMin;

			hit.Point = ray.Origin + ray.Direction * tMin;
			hit.Normal = glm::transpose(inverse) * glm::vec4(GetNormal(hit.Point), 0.0f);
			hit.Normal = glm::normalize(hit.Normal);
			
			return true;
		}

		return false;
	}

	glm::vec3 Box::GetNormal(const glm::vec3& point)
	{
		const glm::vec3 localPoint = glm::vec3(glm::inverse(m_transform.GetMatrix()) * glm::vec4(point, 1.0f));

		if (abs(localPoint.x - m_bounds[0].x) < 0.01f)
		{
			return glm::vec3(-1.0f, 0.0f, 0.0f);
		}
		
		if (abs(localPoint.x - m_bounds[1].x) < 0.01f)
		{
			return glm::vec3(1.0f, 0.0f, 0.0f);
		}

		if (abs(localPoint.y - m_bounds[0].y) < 0.01f)
		{
			return glm::vec3(0.0f, -1.0f, 0.0f);
		}
		
		if (abs(localPoint.y - m_bounds[1].y) < 0.01f)
		{
			return glm::vec3(0.0f, 1.0f, 0.0f);
		}

		if (abs(localPoint.z - m_bounds[0].z) < 0.01f)
		{
			return glm::vec3(0.0f, 0.0f, -1.0f);
		}
		
		if (abs(localPoint.z - m_bounds[1].z) < 0.01f)
		{
			return glm::vec3(0.0f, 0.0f, 1.0f);
		}

		return glm::vec3(0.0f);
	}
}