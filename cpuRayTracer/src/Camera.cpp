#include "Camera.h"

namespace CRT
{
	Camera::Camera() :
		m_fovInRad(glm::radians<float>(70.0f)),
		m_viewPortPixelWidth(0),
		m_viewPortPixelHeight(0),
		m_zNear(0.1f),
		m_zFar(1.0f),
		m_aspectRatio(1.2f),
		m_cameraSize(10.0f),
		m_aperture(0.0f)
	{}

	void Camera::Initialize(int pixelWidth, int pixelHeight, float zNear, float zFar)
	{
		m_viewPortPixelWidth = pixelWidth;
		m_viewPortPixelHeight = pixelHeight;
		m_zNear = zNear;
		m_zFar = zFar;
		m_aspectRatio = static_cast<float>(pixelWidth) / static_cast<float>(pixelHeight);
	}

	void Camera::SetFieldOfView(float fovInDeg)
	{
		m_fovInRad = glm::radians<float>(fovInDeg);
	}

	float Camera::GetAspectRatio() const
	{
		return m_aspectRatio;
	}

	float Camera::GetFovInRads() const
	{
		return m_fovInRad;
	}

	float Camera::GetZNear() const
	{
		return m_zNear;
	}

	float Camera::GetZFar() const
	{
		return m_zFar;
	}

	void Camera::SetAperture(float aperture)
	{
		m_aperture = aperture;
	}

	float Camera::GetAperture() const
	{
		return m_aperture;
	}

	void Camera::SetFocalLength(float focalLength)
	{
		m_focalLength = focalLength;
	}

	float Camera::GetFocalLength() const
	{
		return m_focalLength;
	}

	void Camera::SetCameraSize(float size)
	{
		m_cameraSize = size;
	}

	float Camera::GetCameraSize() const
	{
		return m_cameraSize;
	}

	int Camera::GetViewPortPixelWidth() const
	{
		return m_viewPortPixelWidth;
	}

	int Camera::GetViewPortPixelHeight() const
	{
		return m_viewPortPixelHeight;
	}

	Transform& Camera::GetTransform()
	{
		return m_transform;
	}
}