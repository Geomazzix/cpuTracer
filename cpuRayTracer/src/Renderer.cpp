#include "renderer.hpp"
#include <random>

#include "ray.hpp"
#include "scene.hpp"
#include "primitives/primitive.hpp"
#include "transform.hpp"
#include "camera.hpp"
#include "dialectricTable.hpp"
#include "utility/progressReporter.hpp"
#include "light.hpp"
#include "image.hpp"
#include "material.hpp"
#include "jobsystem.hpp"
#include "spacialSubdivision/bvh.hpp"

//#define NORMAL_COLORING

namespace CRT
{
	Renderer::Renderer() : 
		m_imagePixels(nullptr),
		m_jobSystem(),
		m_skySphere()
	{}

	Renderer::~Renderer()
	{
		delete[] m_imagePixels;
	}

	void Renderer::Initialize(std::shared_ptr<JobSystem> jobSystem, int numPixels, const RendererConfig& renderConfig)
	{
		m_config = renderConfig;
		m_jobSystem = jobSystem;
		m_imagePixels = new glm::vec3[numPixels];
	}

	glm::vec3 Renderer::ComputeRayColor(const Ray& ray, Scene& scene, float maxRayLength, float& absorbDistance, int depth)
	{
		HitInfo hitInfo;
		const bool intersectResult = scene.Intersect(ray, hitInfo, maxRayLength);

		if (!intersectResult)
		{
			return scene.GetSkySphere()->SampleSky(ray);
		}

#if defined RENDER_BVH_HEATMAP
		static constexpr std::array<glm::vec3, 4> heatmapColors = {
			glm::vec3(0.0f, 0.0f, 1.0f), /* Blue */
			glm::vec3(0.0f, 1.0f, 1.0f), /* Cyan */
			glm::vec3(0.0f, 1.0f, 0.0f), /* Green */
			glm::vec3(1.0f, 1.0f, 0.0f)  /* Yellow */
		};

		const float t = glm::clamp(static_cast<float>(hitInfo.TraversalSteps) / 300.f, 0.0f, 1.0f) * (heatmapColors.size() - 1);
		const int index = glm::clamp(static_cast<int>(t), 0, static_cast<int>(heatmapColors.size() - 2));
		return glm::mix(heatmapColors[index], heatmapColors[index + 1], t - index);
#endif

		auto material = hitInfo.HitPrimitive->GetMaterial().lock();
		if (material->IsDialetic)
		{
			return depth < m_config.MaxRenderDepth
				? ApplyRefraction(ray, scene, hitInfo, maxRayLength, absorbDistance, depth)
				: glm::vec3(0.f);
		}

		const float reflectivity = material->IsReflective ? material->Reflectivity : 0.0f;
		glm::vec3 color = (1.0f - reflectivity) * ApplyLighting(ray, scene, material, hitInfo.Point, hitInfo.Normal, maxRayLength);

		if (material->IsReflective && depth < m_config.MaxRenderDepth)
		{
			const Ray reflectionRay = 
			{
				.Origin = hitInfo.Point + hitInfo.Normal * 1e-4f,
				.Direction = glm::reflect(ray.Direction, hitInfo.Normal),
				.IsDialectricPath = ray.IsDialectricPath
			};
			color += reflectivity * ComputeRayColor(reflectionRay, scene, maxRayLength, absorbDistance, depth + 1);
		}

		return color;
	}

	glm::vec3 Renderer::ApplyRefraction(const Ray& ray, Scene& scene, const HitInfo& hitInfo, float maxRayLength, float& absorbDistance, int depth)
	{
		auto material = hitInfo.HitPrimitive->GetMaterial().lock();

		/* #Todo: Add proper medium tracing so nested mediums can be traced, i.e. water in glass in "air". */
		float n1 = DialetricIndexTable::GetDialetricIndex(DialetricType::Air);
		float n2 = material->RefractiveIndex;
		auto attenuation = glm::vec3(1.0f);
		auto normal = hitInfo.Normal;

		/* Check if this ray is exiting the medium, if ensure that the absorbance is calculated. If it isn't this can be ignored as the radiance is not taken into account for the current pixel (yet?) */
		if (glm::dot(ray.Direction, normal) > 0.0f)
		{
			const auto absorbance = material->AbsorbanceCoefficient * (hitInfo.Distance + absorbDistance);
			attenuation = { expf(-absorbance.x), expf(-absorbance.y) , expf(-absorbance.z) };
			normal = -hitInfo.Normal;
			std::swap(n1, n2);
		}

		/* Internal/Outgoing reflection */
		auto reflectionAbsorbtion = absorbDistance;
		const auto reflectionDirection = glm::reflect(ray.Direction, normal);
		const Ray reflectionRay =
		{
			.Origin = hitInfo.Point + reflectionDirection * 1e-4f,
			.Direction = reflectionDirection,
			.IsDialectricPath = true
		};
		const auto reflectionColor = ComputeRayColor(reflectionRay, scene, maxRayLength, reflectionAbsorbtion, depth + 1);

		/* Internal/Outgoing refraction */
		auto refractionAbsorbtion = absorbDistance;
		const auto refractionDirection = glm::refract(ray.Direction, normal, n1 / n2);
		const Ray refractionRay =
		{
			.Origin = hitInfo.Point + refractionDirection * 1e-4f,
			.Direction = refractionDirection,
			.IsDialectricPath = true
		};
		const auto refractionColor = ComputeRayColor(refractionRay, scene, maxRayLength, refractionAbsorbtion, depth + 1);

		/* Compute the full light accumalation of the dialectric ray on this intersection point. */
		const auto reflectionMultiplier = SchlickApproximation(n1, n2, hitInfo.Normal, ray.Direction, material->Reflectivity);
		return attenuation * (reflectionMultiplier * reflectionColor + (1.0f - reflectionMultiplier) * refractionColor);
	}

	glm::vec3 Renderer::ApplyLighting(const Ray& ray, Scene& scene, std::shared_ptr<Material> material, const glm::vec3& hitPoint, const glm::vec3& normal, float maxRayLength)
	{
		glm::vec3 pixelColor = glm::vec3(0.0f);
		for (int i = 0; i < scene.GetLightCount(); i++)
		{
			Light& pointLight = scene.GetLight(i);
			glm::vec3 lightPosition = pointLight.GetTransform().GetTranslation();

			HitInfo shadowHitInfo;
			Ray shadowRay;
			glm::vec3 shadowRayDirection = glm::normalize(lightPosition - hitPoint);
			shadowRay.Origin = hitPoint + shadowRayDirection * 1e-2f;
			shadowRay.Direction = shadowRayDirection;

			float lightDistance = glm::length(lightPosition - hitPoint);
			if (m_config.EnableShadows && scene.Intersect(shadowRay, shadowHitInfo, lightDistance)) 
			{
				auto shadowMat = shadowHitInfo.HitPrimitive->GetMaterial().lock();
				if (!shadowMat->IsDialetic) 
				{
					continue;
				}
			}

			glm::vec3 lightDir = glm::vec3(0.0f);
			float lightingAttentuation = 0.0f;

			switch(scene.GetLight(i).GetType())
			{
			case LightType::Directional:
			{
				lightDir = scene.GetLight(i).GetTransform().GetMatrix()[3];
				lightingAttentuation = scene.GetLight(i).GetIntensity();
				break;
			}
			case LightType::Point:
			{
				lightDir = glm::normalize(lightPosition - hitPoint);
				lightingAttentuation = pointLight.CalculateLightingAttentuation(hitPoint);
				break;
			}
			default:
				printf("ERROR: Unsupported light type is being used in the renderer, this could lead to incorrect render results!\n");
				break;
			}

			glm::vec3 halfwayVector = glm::normalize(-ray.Direction + lightDir);
			glm::vec3 diffuse = material->AlbedoCoefficient * lightingAttentuation * std::max(glm::dot(normal, lightDir), 0.0f);
			glm::vec3 specular = material->SpecularCoefficient * lightingAttentuation * std::powf(std::max(glm::dot(normal, halfwayVector), 0.0f), static_cast<float>(material->PhongExponent));

			pixelColor += diffuse + specular;
		}

		return pixelColor + material->AlbedoCoefficient * scene.GetAmbientLight().Intensity;
	}

	glm::vec3* Renderer::Render(Camera& camera, Scene& scene)
	{
		if (m_imagePixels == nullptr)
		{
			printf("ERROR: The renderer has not been initialized yet!");
			return nullptr;
		}

		const glm::mat4 cameraWorldMatrix = camera.GetTransform().GetMatrix();
		const glm::vec3 cameraPosition = cameraWorldMatrix[3];

		const float maxRayLength = camera.GetZFar();
		const float aspectRatio = camera.GetAspectRatio();
		const int width = camera.GetViewPortPixelWidth();
		const int height = camera.GetViewPortPixelHeight();
		const float imagePlaneScale = std::tanf(camera.GetFovInRads() / 2.0f);

		const glm::ivec2 clusterSize = glm::ivec2(4, 4);

		auto ACESFilm = [](glm::vec3 x) 
		{
			const float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
			return glm::clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0f, 1.0f);
		};

		printf("\nRender starting...\n");
		ProgressReporter reporter(m_jobSystem, static_cast<uint64_t>(width * height), "RenderReporter");

		for (int y = 0; y < height; y += clusterSize.y)
		{
			for (int x = 0; x < width; x += clusterSize.x)
			{
				ThreadJob job = [&, clusterSize, x, y]()
				{
					for (int i = 0; i < clusterSize.x; i++)
					{
						for (int j = 0; j < clusterSize.y; j++)
						{
							int px = x + i;
							int py = y + j;

							glm::vec2 ndc = {
								-(2.0f * (px + 0.5f) / width - 1.0f) * aspectRatio,
								1.0f - 2.0f * (py + 0.5f) / height
							};

							Ray ray;
							ray.Origin = cameraPosition;
							ray.Direction = glm::normalize(cameraWorldMatrix * glm::vec4(
								ndc.x * imagePlaneScale,
								ndc.y * imagePlaneScale,
								1.0f, 0.0f
							));

							float absorbDistance = 0.0f;
							glm::vec3 color = ComputeRayColor(ray, scene, maxRayLength, absorbDistance);
							glm::vec3 ldr = ACESFilm(color);

							m_imagePixels[px + py * width] = glm::vec3(
								std::powf(ldr.r, 1.0f / 2.2f),
								std::powf(ldr.g, 1.0f / 2.2f),
								std::powf(ldr.b, 1.0f / 2.2f)
							);
						}
					}
				};

				m_jobSystem->Execute(job);
				reporter.Update(clusterSize.x * clusterSize.y);
			}
		}

		reporter.Done();
		m_jobSystem->Wait();

		return m_imagePixels;
	}

	float Renderer::SchlickApproximation(float n1, float n2, const glm::vec3& normal, const glm::vec3& incident, float reflectivity)
	{
		float cosTheta = std::abs(glm::dot(normal, incident));
		float r0 = (n1 - n2) / (n1 + n2);
		r0 *= r0;

		if (n1 > n2)
		{
			float n = n1 / n2;
			float sinT2 = n * n * (1.0f - cosTheta * cosTheta);

			//Total Internal Reflection.
			if (sinT2 > 1.0f)
			{
				return 1.0f;
			}
			cosTheta = sqrt(1.0f - sinT2);
		}

		float x = 1.0f - cosTheta;
		float ret = r0 + (1.0f - r0) * (x * x * x * x * x);
		return (reflectivity + (1.0f - reflectivity) * ret);
	}
}