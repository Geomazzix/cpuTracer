#include "primitives/triangle.hpp"
#include "ray.hpp"
#include "utility/math.hpp"

namespace CRT
{
	Triangle::Triangle(const std::array<Vertex, 3>& vertexAttributes) :
		Primitive(),
		m_vertexAttributes(vertexAttributes)
	{
		glm::vec3 minBounds = glm::vec3(INFINITY);
		glm::vec3 maxBounds = glm::vec3(-INFINITY);
		minBounds = Min(minBounds, m_vertexAttributes[0].Position);
		minBounds = Min(minBounds, m_vertexAttributes[1].Position);
		minBounds = Min(minBounds, m_vertexAttributes[2].Position);

		maxBounds = Max(maxBounds, m_vertexAttributes[0].Position);
		maxBounds = Max(maxBounds, m_vertexAttributes[1].Position);
		maxBounds = Max(maxBounds, m_vertexAttributes[2].Position);

		m_aabb = AABB(minBounds, maxBounds);
	}

	bool Triangle::Intersect(const Ray& ray, HitInfo& hitInfo, const float maxRayLength)
	{
		glm::vec3 vertex0 = m_vertexAttributes[0].Position;
		glm::vec3 vertex1 = m_vertexAttributes[1].Position;
		glm::vec3 vertex2 = m_vertexAttributes[2].Position;

		glm::vec3 edge1 = vertex1 - vertex0;
		glm::vec3 edge2 = vertex2 - vertex0;
		
		glm::vec3 h = glm::cross(ray.Direction, edge2);
		float a = glm::dot(edge1, h);
		if (a > -0.000001f && a < 0.000001f)
		{
			// This ray is parallel to this triangle.
			return false;   
		}

		float f = 1.f / a;
		glm::vec3 s = ray.Origin - vertex0;
		float u = f * glm::dot(s, h);
		if (u < 0.f || u > 1.f)
		{
			return false;
		}
		
		glm::vec3 q = glm::cross(s, edge1);
		float v = f * glm::dot(ray.Direction, q);
		if (v < 0.f || (u + v) > 1.f)
		{
			return false;
		}

		// At this stage we can compute t to find out where the intersection point is on the line.
		float t = f * glm::dot(edge2, q);
		if (t > 0.000001f && t < maxRayLength)
		{
			hitInfo.Distance = t;
			hitInfo.HitPrimitive = this;
			hitInfo.Normal = glm::normalize(
				(1.0f - u - v) * m_vertexAttributes[0].Normal 
				+ u * m_vertexAttributes[1].Normal 
				+ v * m_vertexAttributes[2].Normal
			);
			return true;
		}
		else
		{
			// This means that there is a line intersection but not a ray intersection.
			return false;
		}
	}
}