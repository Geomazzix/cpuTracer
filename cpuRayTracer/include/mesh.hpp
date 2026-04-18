#pragma once
#include "spacialSubdivision/bvh.hpp"

namespace CRT
{	
	/**
	 * @brief The mesh represent a number of intersectible triangles and uses a Bounding Volume Hierarchy to improve the intersection performance.
	 * @note The BVH is unique per mesh, it's therefor important to share meshes and not copy them.
	 */
	class Mesh : public Primitive
	{
	public:
		Mesh() = default;
		~Mesh() = default;

		void SetTriangles(std::vector<std::shared_ptr<Primitive>>&& triangles);
		void Delete();

		std::shared_ptr<Primitive> GetTriangle(int index);
		size_t GetTriangleCount() const;

		bool Intersect(const Ray& ray, HitInfo& hitInfo, float maxRayLength);

	private:
		std::vector<std::shared_ptr<Primitive>> m_triangles;
		BVH m_blas;
	};
}