#pragma once
#include <vec3.hpp>
#include <memory>

namespace CRT
{
	struct Ray;

	/**
	 * @brief Heap-stored image, data is always stored in linear color space, always transform to srgb before showing.
	 * @note The image has ownership over the float* Data, and frees it upon deletion.
	 */
	struct Image final
	{
		Image();
		~Image();

		int Width;
		int Height;
		int ChannelCount;
		float* Data;
		bool IsHdr;

		auto LookUp(float u, float v) -> glm::vec3;
	};

	/**
	 * @brief The skysphere is used to sample HDRI maps using a ray.
	 */
	class SkySphere final
	{
	public:
		explicit SkySphere(std::shared_ptr<Image> image);
		explicit SkySphere(const glm::vec3& skyColor = glm::vec3(0.f));
		~SkySphere() = default;

		auto SampleSky(const Ray& ray) const -> glm::vec3;

	private:
		std::shared_ptr<Image> m_skySphere;
		glm::vec3 m_skyColor;
	};
}