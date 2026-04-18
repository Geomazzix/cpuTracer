#include "scene.hpp"
#include "filesystem.hpp"
#include "ray.hpp"
#include "spacialSubdivision/bvh.hpp"
#include "image.hpp"
#include "primitives/primitive.hpp"
#include "light.hpp"

namespace CRT
{
	Scene::Scene() :
		m_skySphere(nullptr)
	{
		m_ambientLight = std::make_shared<AmbientLight>();
		m_skySphere = std::make_unique<SkySphere>();
	}

	Scene::~Scene() = default;

	int Scene::AddLight(std::shared_ptr<Light> light)
	{
		m_directionalLights.push_back(light);
		return static_cast<int>(m_directionalLights.size() - 1);
	}

	int Scene::AddPrimitive(std::shared_ptr<Primitive> primitive)
	{
		m_primitives.push_back(primitive);
		return static_cast<int>(m_primitives.size() - 1);
	}

	void Scene::SetAmbientIntensity(float intensity)
	{
		m_ambientLight->Intensity = intensity;
	}

	void Scene::LoadSkySphere(const std::filesystem::path& hdriFilePath)
	{
		auto hdriMap = IO::LoadImage(hdriFilePath.string().c_str());
		m_skySphere = std::make_unique<SkySphere>(hdriMap);
	}

	const SkySphere* Scene::GetSkySphere()
	{
		return m_skySphere.get();
	}

	Light& Scene::GetLight(int dataIndex)
	{
		return *m_directionalLights[dataIndex].get();
	}

	size_t Scene::GetLightCount()
	{
		return m_directionalLights.size();
	}

	Primitive& Scene::GetPrimitive(int dataIndex)
	{
		return *m_primitives[dataIndex].get();
	}

	size_t Scene::GetPrimitiveCount()
	{
		return m_primitives.size();
	}

	AmbientLight& Scene::GetAmbientLight()
	{
		return *m_ambientLight;
	}

	void Scene::GenerateBVH()
	{
		m_tlas = std::make_unique<BVH>();
		
		const BVHConfig bvhConfig =
		{
			.PartitionType = PartitionType::SurfaceAreaHeuristic
		};

		m_tlas->Initialize(bvhConfig, m_primitives);
	}

	bool Scene::Intersect(const Ray& ray, HitInfo& lastHitInfo, float maxRayLength)
	{
		assert(m_tlas);
		return m_tlas->Intersect(ray, lastHitInfo, maxRayLength);
	}

	void Scene::ShutDown()
	{
		m_tlas->Shutdown();
	}
}