#pragma once
#include <vec3.hpp>
#include <memory>
#include "transform.hpp"

namespace CRT
{
	/**
	 * @brief The ambient light replaces indirect illumination for the time being.
	 * @note Physically accurate indirect illumination would require the transformation from a whitted
	 * style raytracer to a path tracer.
	 */
	struct AmbientLight final
	{
		float Intensity = 0.3f;
	};

	/**
	 * @brief Identifies the different light types.
	 */
	enum class LightType : uint8_t
	{
		Directional = 0,
		Point
	};

	/**
	 * @brief The base light class represents a directional light, this can be overwritten by other light types as the attenuation can be adjusted.
	 */
	class Light
	{
	public:
		Light();
		virtual ~Light() = default;

		void Initialize(LightType type, const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& eulerAngles = glm::vec3(0.0f), const glm::vec3& scale = glm::vec3(1.0f));

		void SetIntensity(float intensity);
		float GetIntensity() const;

		LightType GetType() const;
		Transform& GetTransform();

		/* Based on the linear + quadratic lighting attentuation from Blender.
		 * https://docs.blender.org/manual/fr/2.79/render/blender_render/lighting/lights/attenuation.html */
		virtual float CalculateLightingAttentuation(const glm::vec3& point) = 0;

	protected:
		Transform m_transform;
		float m_intensity;
		LightType m_type;
	};
	
	/**
	 * @brief Point light implementation, representing a point of light in the sky.
	 * @note Not physically accurate, the concept of a point of light doesn't exist, this should technically always be sampled from a surface.
	 * @note Not marked *final*, a spot light could use these properties.
	 */
	class PointLight : public Light
	{
	public:
		PointLight();
		virtual ~PointLight() = default;

		void SetRange(float radius);
		float GetRange() const;

		float CalculateLightingAttentuation(const glm::vec3& point) override;

	protected:
		float m_range;
	};
}