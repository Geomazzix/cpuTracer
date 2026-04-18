#include "primitives/primitive.hpp"
#include "ray.hpp"

namespace CRT
{
	Primitive::Primitive() :
		m_transform(),
		m_material()
	{}

	void Primitive::Initialize(std::weak_ptr<Material> material, const glm::vec3& position, const glm::vec3& eulerAngles, const glm::vec3& scale, const AABB& aabb)
	{
		m_material = material;
		m_transform.Translate(position);
		m_transform.Rotate(eulerAngles);
		m_transform.Scale(scale);
		m_aabb = aabb;
	}

	void Primitive::SetMaterial(std::weak_ptr<Material>& material)
	{
		m_material = material;
	}

	const Transform& Primitive::GetTransform() const
	{
		return m_transform;
	}

	std::weak_ptr<Material> Primitive::GetMaterial()
	{
		return m_material;
	}

	const AABB& Primitive::GetAabb() const
	{
		return m_aabb;
	}

	AABB Primitive::GetWorldAabb()
	{
		const glm::mat4 matrix = m_transform.GetMatrix();
		const glm::vec3 xa = glm::vec3(matrix[0]) * m_aabb.GetMin().x;
		const glm::vec3 xb = glm::vec3(matrix[0]) * m_aabb.GetMax().x;

		const glm::vec3 ya = glm::vec3(matrix[1]) * m_aabb.GetMin().y;
		const glm::vec3 yb = glm::vec3(matrix[1]) * m_aabb.GetMax().y;

		const glm::vec3 za = glm::vec3(matrix[2]) * m_aabb.GetMin().z;
		const glm::vec3 zb = glm::vec3(matrix[2]) * m_aabb.GetMax().z;

		return AABB(
			glm::min(xa, xb) + glm::min(ya, yb) + glm::min(za, zb) + m_transform.GetTranslation(),
			glm::max(xa, xb) + glm::max(ya, yb) + glm::max(za, zb) + m_transform.GetTranslation()
		);
	}
}