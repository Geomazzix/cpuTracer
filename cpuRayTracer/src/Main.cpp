#include <memory>
#include <glm.hpp>
#include "filesystem.hpp"
#include "resourceCache.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "mesh.hpp"
#include "material.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "primitives/plane.hpp"
#include "primitives/sphere.hpp"
#include "primitives/box.hpp"
#include "dialectricTable.hpp"
#include "jobsystem.hpp"

using namespace CRT;

const glm::uvec2 imageDimensions = { 3840, 2160 };

int main(int argc, char** argv)
{
	std::shared_ptr<JobSystem> jobSystem = std::make_shared<JobSystem>();
	jobSystem->Initialize();

	/* 1. Resource loading and scene setup. */
	ResourceManager resourceManager;
	resourceManager.Initialize();

	std::shared_ptr<Material> groundMaterial = std::make_shared<Material>();
	groundMaterial->AlbedoCoefficient = glm::vec3(0.7f);
	groundMaterial->SpecularCoefficient = glm::vec3(0.5f);
	groundMaterial->PhongExponent = 100;
	groundMaterial->IsReflective = true;
	groundMaterial->Reflectivity = 1.0f;
	resourceManager.AddMaterial("GroundMaterial", groundMaterial);
	
	Scene scene;
	scene.SetAmbientIntensity(0.2f);
	scene.LoadSkySphere(std::filesystem::current_path() / "res/old_hall_4k.hdr");

	std::shared_ptr<Plane> groundPlane = std::make_shared<Plane>();
	groundPlane->Initialize(groundMaterial, glm::vec3(0.0f, 4.f, 0.0f), glm::vec3(0.0f), glm::vec3(1000, 0.001f, 1000));
	scene.AddPrimitive(groundPlane);

	//std::shared_ptr<Mesh> mesh = IO::LoadWavefrontFile(resourceManager, "res/models/stanford/stanford-bunny.obj");
	//std::shared_ptr<Mesh> mesh = IO::LoadWavefrontFile(resourceManager, "res/models/stanford/cow.obj");
	std::shared_ptr<Mesh> mesh = IO::LoadWavefrontFile(resourceManager, "res/models/stanford/xyzrgb_dragon_smoothed.obj");
	scene.AddPrimitive(mesh);

	std::shared_ptr<PointLight> mainLight = std::make_shared<PointLight>();
	mainLight->Initialize(LightType::Point, glm::vec3(150.0f, 300.0f, -50.0f));
	mainLight->SetRange(500.0f);
	mainLight->SetIntensity(0.8f);
	scene.AddLight(mainLight);

	scene.GenerateBVH();

	/* 2. Render setup and rendering he image. */
	Camera camera;
	camera.Initialize(imageDimensions.x, imageDimensions.y);
	camera.GetTransform().Translate(glm::vec3(175.f, 175.f, 175.f));
	camera.GetTransform().LookAt(glm::vec3(0.0f, 25.0f, 0.0f));
	camera.SetFieldOfView(50.0f);
	camera.SetAperture(1.5f);
	camera.SetFocalLength(100.0f);

	const RendererConfig config =
	{
		.MaxRenderDepth = 10,
		.EnableShadows = true
	};

	Renderer renderer;
	renderer.Initialize(jobSystem, imageDimensions.x * imageDimensions.y, config);

	glm::vec3* colorData = renderer.Render(camera, scene);
	printf("Done processing the image, now image output starting!\n");
	IO::StorePNG("RenderResult.png", imageDimensions.x, imageDimensions.y, 3, colorData, true);

	return 0;
}