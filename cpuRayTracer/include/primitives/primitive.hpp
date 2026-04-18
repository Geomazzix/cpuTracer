#pragma once
#include <vec3.hpp>
#include <memory>
#include "spacialSubdivision/aabb.hpp"
#include "transform.hpp"

namespace CRT
{
	struct Material;
	struct Ray;
	struct HitInfo;

	/**
	 * @brief The primitive base class serves as the common base class for anything that has a position, orientation and scale and is able to
	 * be intersected using a ray.
	 */
	class Primitive
	{
	public:
		Primitive();
		virtual ~Primitive() = default;

		virtual void Initialize(std::weak_ptr<Material> material, 
			const glm::vec3& position = glm::vec3(0.0f), 
			const glm::vec3& eulerAngles = glm::vec3(0.0f), 
			const glm::vec3& scale = glm::vec3(1.0f), 
			const AABB& aabb = AABB());
		virtual bool Intersect(const Ray& ray, HitInfo& hit, const float maxRayLength) = 0;

		void SetMaterial(std::weak_ptr<Material>& material);
		std::weak_ptr<Material> GetMaterial();
		const Transform& GetTransform() const;

		const AABB& GetAabb() const;
		AABB GetWorldAabb();

	protected:
		Transform m_transform;
		AABB m_aabb;
		std::weak_ptr<Material> m_material;
	};
}