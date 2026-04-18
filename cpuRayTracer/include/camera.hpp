#pragma once
#include "transform.hpp"

namespace CRT
{
	/**
	 * @brief The camera class is used to define the location and size proportions of the image being rendered by the renderer.
	 */
	class Camera final
	{
	public:
		Camera();
		~Camera() = default;

		void Initialize(int pixelWidth, int pixelHeight, float zNear = 0.1f, float zFar = 1000.0f);
		
		void SetFieldOfView(float fovInDeg);
		float GetFovInRads() const;

		float GetAspectRatio() const;

		float GetZNear() const;
		float GetZFar() const;

		void SetAperture(float aperture);
		float GetAperture() const;

		void SetFocalLength(float focalLength);
		float GetFocalLength() const;

		void SetCameraSize(float size);
		float GetCameraSize() const;

		int GetViewPortPixelWidth() const;
		int GetViewPortPixelHeight() const;

		Transform& GetTransform();

	private:
		Transform m_transform;

		int m_viewPortPixelWidth;
		int m_viewPortPixelHeight;
		float m_aspectRatio;
		float m_aperture;

		float m_cameraSize;
		float m_fovInRad;
		float m_focalLength;

		float m_zNear;
		float m_zFar;
	};
}