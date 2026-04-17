#include "Image.h"
#include <cmath>
#include <algorithm>
#include "Ray.h"
#include <ext/scalar_constants.hpp>
#include <gtc/constants.hpp>

namespace CRT
{
	Image::Image() :
		Width(0),
		Height(0),
		ChannelCount(0),
		Data(nullptr),
		IsHdr(false)
	{}

	Image::~Image()
	{
		if (Data)
		{
			delete Data;
		}
	}

	glm::vec3 Image::LookUp(float u, float v)
	{
		const auto x = std::clamp<int>(static_cast<int>(std::round(u * static_cast<float>(Width) - 0.5f)), 0, Width - 1);
		const auto y = std::clamp<int>(static_cast<int>(std::round(v * static_cast<float>(Height) - 0.5f)), 0, Height - 1);
		const auto idx = (y * Width + x) * 3;
		return glm::vec3{ Data[idx], Data[idx + 1], Data[idx + 2] };
	}

	SkySphere::SkySphere(std::shared_ptr<Image> image) :
		m_skySphere(image),
		m_skyColor(0)
	{}

	SkySphere::SkySphere(const glm::vec3& skyColor) :
		m_skySphere(nullptr),
		m_skyColor(skyColor)
	{}

	auto SkySphere::SampleSky(const Ray& ray) const -> glm::vec3
	{
		if (!m_skySphere)
		{
			return m_skyColor;
		}

		const auto u = 1 - (atan2(ray.Direction.z, ray.Direction.x) + glm::pi<float>()) / (glm::two_pi<float>());
		const auto v = (asin(ray.Direction.y) + glm::half_pi<float>()) / glm::pi<float>();
		auto skyColor = m_skySphere->LookUp(u, v);

		if (ray.IsDialectricPath)
		{
			const float maxComponent = std::max({ skyColor.r, skyColor.g, skyColor.b });
			if (maxComponent > 1.0f)
			{
				skyColor *= 1.0f / maxComponent;
			}
		}

		return skyColor;
	}
}