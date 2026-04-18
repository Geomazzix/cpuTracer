#pragma once
#include <vec3.hpp>
#include <memory>

namespace CRT
{
	struct Ray;
	struct HitInfo;
	struct Image;
	struct Material;
	class Scene;
	class JobSystem;
	class Camera;

	/**
	 * @brief Configuration used by the renderer when rendering the image.
	 */
	struct RendererConfig
	{
		int MaxRenderDepth = 4;
		bool EnableShadows = true;
	};

	/**
	 * @brief The renderer renders the provided scene using the provide camera into heap allocated vector of colors. 
	 */
	class Renderer final
	{
	public:
		Renderer();
		~Renderer();

		void Initialize(std::shared_ptr<JobSystem> jobSystem, int numPixels, const RendererConfig& renderConfig);
		glm::vec3* Render(Camera& camera, Scene& scene);

	private:
		glm::vec3 ComputeRayColor(const Ray& ray, Scene& scene, float maxRayLength, float& absorbDistance, int depth = 0);
		glm::vec3 ApplyRefraction(const Ray& ray, Scene& scene, const HitInfo& hitInfo, float maxRayLength, float& absorbDistance, int depth = 0);
		glm::vec3 ApplyLighting(const Ray& ray, Scene& scene, std::shared_ptr<Material> material, const glm::vec3& hitPoint, const glm::vec3& normal, float maxRayLength);
		float SchlickApproximation(float n1, float n2, const glm::vec3& normal, const glm::vec3& incident, float reflectivity);

		RendererConfig m_config;
		std::shared_ptr<JobSystem> m_jobSystem;
		std::weak_ptr<Image> m_skySphere;
		glm::vec3* m_imagePixels;
	};
}