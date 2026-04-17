#include "Light.h"

namespace CRT
{
	Light::Light() :
		m_intensity(1.0f),
		m_type(LightType::Directional),
		m_transform()
	{}

	void Light::Initialize(LightType type, const glm::vec3& position, const glm::vec3& eulerAngles, const glm::vec3& scale)
	{
		m_transform.Translate(position);
		m_transform.Rotate(eulerAngles);
		m_transform.Scale(scale);
		m_type = type;
	}

	Transform& Light::GetTransform()
	{
		return m_transform;
	}

	void Light::SetIntensity(float intensity)
	{
		m_intensity = intensity;
	}

	float Light::GetIntensity() const
	{
		return m_intensity;
	}


	LightType Light::GetType() const
	{
		return m_type;
	}

	PointLight::PointLight() :
		Light(),
		m_range(10.0f)
	{}

	void PointLight::SetRange(float radius)
	{
		m_range = radius;
	}

	float PointLight::GetRange() const
	{
		return m_range;
	}

	float PointLight::CalculateLightingAttentuation(const glm::vec3& point)
	{
		float distance = glm::length(m_transform.GetTranslation() - point);

		float E = m_intensity;
		float DD = m_range * m_range;
		float Q = 1;
		float rr = distance * distance;

		return E * (DD / (DD + Q * rr));
	}
}