#include "transform.hpp"
#include <gtx/transform.hpp>

namespace CRT
{
	Transform::Transform() :
		m_translation(glm::vec3(0.0f)),
		m_orientation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
		m_scale(glm::vec3(1.0f)),
		m_isDirty(true),
		m_localTransform(glm::identity<glm::mat4>())
	{}

	const glm::mat4& Transform::GetMatrix()
	{
		if (m_isDirty)
			UpdateLocalTransform();
		return m_localTransform;
	}

	void Transform::LookAt(const glm::vec3& targetPoint)
	{
		glm::vec3 localUp = glm::vec3(m_localTransform[1].x, m_localTransform[1].y, m_localTransform[1].z);
		glm::vec3 direction = glm::normalize(targetPoint - m_translation);
		m_orientation = glm::quatLookAtLH(direction, localUp);
		m_isDirty = true;
	}

	void Transform::Translate(const glm::vec3& translation)
	{
		m_translation += translation;
		m_isDirty = true;
	}

	void Transform::Rotate(const glm::vec3& eulerAngles)
	{
		Rotate(glm::quat(eulerAngles));
	}

	void Transform::Rotate(const glm::quat& quaternion)
	{
		m_orientation *= quaternion;
		m_isDirty = true;
	}

	void Transform::Scale(const glm::vec3& scale)
	{
		m_scale *= scale;
		m_isDirty = true;
	}

	glm::vec3 Transform::GetTranslation() const
	{
		return m_translation;
	}

	glm::quat Transform::GetRotation() const
	{
		return m_orientation;
	}

	glm::vec3 Transform::GetScale() const
	{
		return m_scale;
	}

	void Transform::UpdateLocalTransform()
	{
		glm::mat4 translation = glm::translate(glm::identity<glm::mat4>(), m_translation);
		glm::mat4 orientation = glm::toMat4(m_orientation);
		glm::mat4 scale = glm::scale(glm::identity<glm::mat4>(), m_scale);

		m_localTransform = translation * orientation * scale;
		m_isDirty = false;
	}
}