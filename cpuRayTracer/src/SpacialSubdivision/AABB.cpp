#include "spacialSubdivision/aabb.hpp"
#include "ray.hpp"
#include "utility/math.hpp"

namespace CRT
{
	AABB::AABB(const glm::vec3& min /*= glm::vec3(-1.0f)*/, const glm::vec3& max /*= glm::vec3(1.0f)*/)
	{
		m_bounds[0] = min;
		m_bounds[1] = max;
	}

	bool AABB::Intersect(const Ray& ray, const glm::vec3& inverseDirection, const std::array<int, 3> directionIsNegative, const float maxRayLength) const
	{
		float tMin = (m_bounds[directionIsNegative[0]].x - ray.Origin.x) * inverseDirection.x;
		float tMax = (m_bounds[1 - directionIsNegative[0]].x - ray.Origin.x) * inverseDirection.x;
		float tyMin = (m_bounds[directionIsNegative[1]].y - ray.Origin.y) * inverseDirection.y;
		float tyMax = (m_bounds[1 - directionIsNegative[1]].y - ray.Origin.y) * inverseDirection.y;

		if (tMin > tyMax || tyMin > tMax)
		{
			return false;
		}

		if (tyMin > tMin)
		{
			tMin = tyMin;
		}

		if (tyMax < tMax)
		{
			tMax = tyMax;
		}

		float tzMin = (m_bounds[directionIsNegative[2]].z - ray.Origin.z) * inverseDirection.z;
		float tzMax = (m_bounds[1 - directionIsNegative[2]].z - ray.Origin.z) * inverseDirection.z;

		if (tMin > tzMax || tzMin > tMax)
		{
			return false;
		}

		if (tzMin > tMin)
		{
			tMin = tzMin;
		}

		if (tzMax < tMax)
		{
			tMax = tzMax;
		}

		return (tMin < maxRayLength) && (tMax > 0);
	}

#if _DEBUG
	bool AABB::Intersect(const Ray& ray, float& debugTmax, float& debugTmin, glm::vec3& bvhColor, const glm::vec3& inverseDirection, const std::array<int, 3> directionIsNegative, const float maxRayLength) const
	{
		float tMin =  (m_bounds[directionIsNegative[0]].x - ray.Origin.x) * inverseDirection.x;
		float tMax =  (m_bounds[1 - directionIsNegative[0]].x - ray.Origin.x) * inverseDirection.x;
		float tyMin = (m_bounds[directionIsNegative[1]].y - ray.Origin.y) * inverseDirection.y;
		float tyMax = (m_bounds[1 - directionIsNegative[1]].y - ray.Origin.y) * inverseDirection.y;

		if (tMin > tyMax || tyMin > tMax)
		{
			return false;
		}
		
		if (tyMin > tMin)
		{
			tMin = tyMin;
		}
		
		if (tyMax < tMax)
		{
			tMax = tyMax;
		}

		float tzMin = (m_bounds[directionIsNegative[2]].z - ray.Origin.z) * inverseDirection.z;
		float tzMax = (m_bounds[1 - directionIsNegative[2]].z - ray.Origin.z) * inverseDirection.z;

		if (tMin > tzMax || tzMin > tMax)
		{
			return false;
		}

		if (tzMin > tMin)
		{
			tMin = tzMin;
		}
		
		if (tzMax < tMax)
		{
			tMax = tzMax;
		}
		
		if ((tMin > maxRayLength) || (tMax <= 0))
		{
			return false;
		}

		const float lineThickness = 0.075f;
		glm::vec3 hitPoints[2] = 
		{
			ray.Origin + ray.Direction * tMin,
			ray.Origin + ray.Direction * tMax
		};

		for(int pointIndex = 0; pointIndex < 2; pointIndex++)
		{
			for (int i = 0; i < 2; i++)
			{
				if (abs(hitPoints[pointIndex].x - m_bounds[i].x) < lineThickness)
				{
					if (abs(hitPoints[pointIndex].y - m_bounds[i].y) < lineThickness)
					{
						bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
					}

					if (abs(hitPoints[pointIndex].z - m_bounds[i].z) < lineThickness)
					{
						bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
					}
				}

				if (abs(hitPoints[pointIndex].y - m_bounds[i].y) < lineThickness)
				{
					if (abs(hitPoints[pointIndex].x - m_bounds[i].x) < lineThickness)
					{
						bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
					}

					if (abs(hitPoints[pointIndex].z - m_bounds[i].z) < lineThickness)
					{
						bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
					}
				}

				if (abs(hitPoints[pointIndex].z - m_bounds[i].z) < lineThickness)
				{
					if (abs(hitPoints[pointIndex].x - m_bounds[i].x) < lineThickness)
					{
						bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
					}

					if (abs(hitPoints[pointIndex].y - m_bounds[i].y) < lineThickness)
					{
						bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
					}
				}
			}

			if (abs(hitPoints[pointIndex].y - m_bounds[1].y) < lineThickness)
			{
				if (abs(hitPoints[pointIndex].z - m_bounds[0].z) < lineThickness)
				{
					bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
				}

				if (abs(hitPoints[pointIndex].x - m_bounds[0].x) < lineThickness)
				{
					bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
				}
			}

			if (abs(hitPoints[pointIndex].y - m_bounds[0].y) < lineThickness)
			{
				if (abs(hitPoints[pointIndex].z - m_bounds[1].z) < lineThickness)
				{
					bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
				}

				if (abs(hitPoints[pointIndex].x - m_bounds[1].x) < lineThickness)
				{
					bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
				}
			}

			if (abs(hitPoints[pointIndex].z - m_bounds[1].z) < lineThickness)
			{
				if (abs(hitPoints[pointIndex].y - m_bounds[0].y) < lineThickness)
				{
					bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
				}

				if (abs(hitPoints[pointIndex].x - m_bounds[0].x) < lineThickness)
				{
					bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
				}
			}

			if (abs(hitPoints[pointIndex].z - m_bounds[0].z) < lineThickness)
			{
				if (abs(hitPoints[pointIndex].y - m_bounds[1].y) < lineThickness)
				{
					bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
				}

				if (abs(hitPoints[pointIndex].x - m_bounds[1].x) < lineThickness)
				{
					bvhColor = glm::vec3(1.0f, 0.0f, 0.0f);
				}
			}
		}

		debugTmin = tMin;
		debugTmax = tMax;
		return true;
	}
#endif

	const glm::vec3& AABB::GetMin() const
	{
		return m_bounds[0];
	}

	const glm::vec3& AABB::GetMax() const
	{
		return m_bounds[1];
	}

	glm::vec3 AABB::GetCenter() const
	{
		return (m_bounds[1] + m_bounds[0]) * 0.5f;
	}

	glm::vec3 AABB::GetOffset(const glm::vec3& point) const
	{
		glm::vec3 o = point - m_bounds[0];
		
		if(m_bounds[1].x > m_bounds[0].x)
		{
			o.x /= m_bounds[1].x - m_bounds[0].x;
		}

		if (m_bounds[1].y > m_bounds[0].y)
		{
			o.y /= m_bounds[1].y - m_bounds[0].y;
		}

		if (m_bounds[1].z > m_bounds[0].z)
		{
			o.z /= m_bounds[1].z - m_bounds[0].z;
		}

		return o;
	}

	float AABB::GetSurfaceArea() const
	{
		glm::vec3 diagonal = m_bounds[1] - m_bounds[0];
		return 2.0f * (diagonal.x * diagonal.y + diagonal.x * diagonal.z + diagonal.y * diagonal.z);
	}

	AABB AABB::Combine(const AABB& a, const AABB& b)
	{
		const glm::vec3 min = Min(a.GetMin(), b.GetMin());
		const glm::vec3 max = Max(a.GetMax(), b.GetMax());
		return AABB(min, max);
	}

	AABB AABB::Combine(const AABB& a, const glm::vec3& b)
	{
		const glm::vec3 min = Min(a.GetMin(), b);
		const glm::vec3 max = Max(a.GetMax(), b);
		return AABB(min, max);
	}
}