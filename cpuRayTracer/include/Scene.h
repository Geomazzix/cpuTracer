#pragma once
#include <vector>
#include <memory>
#include <filesystem>

namespace CRT
{
	struct AmbientLight;
	struct Ray;
	struct HitInfo;
	class Camera;
	class Primitive;
	class Light;
	class SkySphere;
	class BVH;

	/**
	 * @brief The scene holds all the data that the renderer needs to render the contents to an image.
	 */
	class Scene final
	{
	public:
		Scene();
		~Scene();

		int AddLight(std::shared_ptr<Light> light);
		int AddPrimitive(std::shared_ptr<Primitive> primitive);

		void SetAmbientIntensity(float intensity);
		void LoadSkySphere(const std::filesystem::path& hdriFilePath);
		const SkySphere* GetSkySphere();

		Light& GetLight(int dataIndex);
		size_t GetLightCount();

		Primitive& GetPrimitive(int dataIndex);
		size_t GetPrimitiveCount();

		AmbientLight& GetAmbientLight();

		void GenerateBVH();
		bool Intersect(const Ray& ray, HitInfo& hitInfo, float maxRayLength);
		void ShutDown();

#if defined(_DEBUG)
		void SetBVHDebugLayer(int index);
#endif

	private:
		std::vector<std::shared_ptr<Primitive>> m_primitives;
		std::vector<std::shared_ptr<Light>> m_directionalLights;
		std::unique_ptr<SkySphere> m_skySphere;
		std::shared_ptr<AmbientLight> m_ambientLight;
		std::unique_ptr<BVH> m_tlas;
	};
}