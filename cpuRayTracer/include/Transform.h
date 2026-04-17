#pragma once
#include <vec3.hpp>
#include <mat4x4.hpp>
#include <gtx/quaternion.hpp>

namespace CRT
{
	/**
	 * @brief The transform is used to define child-parent relationships for scene hierarchy transfersal.
	 */
	class Transform final
	{
	public:
		Transform();
		~Transform() = default;
	
		const glm::mat4& GetMatrix();
	
		void LookAt(const glm::vec3& targetPoint);

		void Translate(const glm::vec3& translation);
		void Rotate(const glm::vec3& eulerAngles);
		void Rotate(const glm::quat& quaternion);
		void Scale(const glm::vec3& scale);

		glm::vec3 GetTranslation() const;
		glm::quat GetRotation() const;
		glm::vec3 GetScale() const;

	private:
		bool m_isDirty;

		glm::vec3 m_translation;
		glm::quat m_orientation;
		glm::vec3 m_scale;

		glm::mat4 m_localTransform;

		void UpdateLocalTransform();
	};
}