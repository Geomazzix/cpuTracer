#include "mesh.hpp"
#include "ray.hpp"

namespace CRT
{
	void Mesh::SetTriangles(std::vector<std::shared_ptr<Primitive>>&& triangles)
	{
		m_triangles = triangles;

		BVHConfig config;
		config.BinningCount = 12;
		config.MaxPrimitiveCountInNode = 2;

		m_blas.Initialize(config, m_triangles);
	}

	std::shared_ptr<Primitive> Mesh::GetTriangle(int index)
	{
		return m_triangles[index];
	}

	size_t Mesh::GetTriangleCount() const
	{
		return m_triangles.size();
	}

	void Mesh::Delete()
	{
		m_blas.Shutdown();
	}

	bool Mesh::Intersect(const Ray& ray, HitInfo& hitInfo, float maxRayLength)
	{
		//Transform the ray to object space.
		const glm::mat4 inverse = glm::inverse(m_transform.GetMatrix());
		const Ray localSpaceRay =
		{
			inverse * glm::vec4(ray.Origin, 1.0f),
			inverse * glm::vec4(ray.Direction, 0.0f)
		};

		if(m_blas.Intersect(localSpaceRay, hitInfo, maxRayLength))
		{
			//Calculate the actual intersection point for world space in the mesh instead of the triangle, since the triangle is based on the model space.
			hitInfo.Point = ray.Origin + ray.Direction * hitInfo.Distance;
			return true;
		}

		return false;
	}
}