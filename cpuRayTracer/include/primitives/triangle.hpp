#pragma once
#include <array>
#include "primitives/primitive.hpp"

namespace CRT
{
	struct Ray;
	struct HitInfo;

	/**
	 * @brief A vertex defines the attributes stored on each vertex of a triangle and with that mesh.
	 */
	struct Vertex final
	{
		glm::vec3 Position;
		glm::vec3 Normal;
	};

	/**
	 * @brief The triangle is considered a primitive, as 
	 */
	class Triangle : public Primitive
	{
	public:
		Triangle(const std::array<Vertex, 3>& vertexAttribs);
		~Triangle() = default;

		bool Intersect(const Ray& ray, HitInfo& hitInfo, const float maxRayLength);

	private:
		std::array<Vertex, 3> m_vertexAttributes;
	};
}