#include "IO.h"
#include <vector>
#include <gtc/type_ptr.hpp>
#include <common.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "Material.h"
#include "Utility/MathUtility.h"
#include "ResourceManager.h"
#include "Image.h"
#include "Mesh.h"
#include "Primitives/Triangle.h"

namespace CRT
{
	std::string IO::GetNameFromFilePath(const std::string& filePath)
	{
		std::size_t extentionCaretPos = filePath.find_last_of('.');
		std::size_t meshNameCaretPosEnd = filePath.find_last_of('/');
		const std::size_t extentionStringLength = filePath.size() - extentionCaretPos;
		return filePath.substr(meshNameCaretPosEnd + 1, filePath.size() - meshNameCaretPosEnd - extentionStringLength - 1);
	}

	void IO::StorePNG(const char* filePath, unsigned int width, unsigned int height, unsigned int numChannels, glm::vec3* colorData, bool normalizedColorData)
	{
		//uint8_t is the channel size, since the renderer samples RGB this means that it has to be multiplied by the channel count.
		uint8_t* pixels = new uint8_t[width * height * numChannels];

		for (unsigned int pixelIndex = 0, j = 0; j < height; j++)
		{
			for (unsigned int i = 0; i < width; i++)
			{
				for (unsigned int channelIndex = 0; channelIndex < numChannels; channelIndex++)
				{
					if (normalizedColorData)
					{
						float channelValue = glm::clamp<float>(colorData[pixelIndex / 3][channelIndex], 0.0f, 1.0f);
						pixels[pixelIndex + channelIndex] = static_cast<uint8_t>(channelValue * 255.0f);
					}
					else
					{
						float channelValue = glm::clamp<float>(colorData[pixelIndex / 3][channelIndex], 0.0f, 255.0f);
						pixels[pixelIndex + channelIndex] = static_cast<uint8_t>(channelValue);
					}
				}

				pixelIndex += numChannels;
			}
		}

		stbi_write_png(filePath, width, height, numChannels, pixels, width * numChannels);
		delete[] pixels;
	}

	std::shared_ptr<Image> IO::LoadImage(const char* filePath)
	{
		/* This function will always load an image into linear space, regardless of the file contents provided. */
		auto image = std::make_shared<Image>();

		stbi_set_flip_vertically_on_load(true);
		stbi_ldr_to_hdr_gamma(2.2f);

		image->IsHdr = stbi_is_hdr(filePath);
		image->Data = stbi_loadf(filePath, &image->Width, &image->Height, &image->ChannelCount, STBI_rgb);

		return image;
	}

	std::shared_ptr<Mesh> IO::LoadWavefrontFile(ResourceManager& resourceManager, const std::string& filePath)
	{
		printf("Started loading %s\n", filePath.c_str());

		std::size_t extentionCaretPos = filePath.find_last_of('.');
		std::size_t meshNameCaretPosEnd = filePath.find_last_of('/');

		const std::size_t extentionStringLength = filePath.size() - extentionCaretPos;
		std::string fileName = IO::GetNameFromFilePath(filePath);
		std::string directory = filePath.substr(0, filePath.size() - fileName.size() - extentionStringLength);

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warning, error;

		const bool result = tinyobj::LoadObj(
			&attrib, 
			&shapes, 
			&materials, 
			&warning, 
			&error, 
			filePath.c_str(), 
			directory.c_str(), 
			true, 
			true
		);

		if (!warning.empty())
		{
			printf("TinyObjLoader WARNING: %s\n", warning.c_str());
		}
		
		if (!error.empty())
		{
			printf("TinyObjLoader ERROR: %s\n", error.c_str());
		}

		if (!result)
		{
			printf("Aborting model loading...\n");
			return nullptr;
		}

		std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
		glm::vec3 minBounds = glm::vec3(INFINITY);
		glm::vec3 maxBounds = glm::vec3(-INFINITY);
		std::vector<std::shared_ptr<Primitive>> triangles; /* What the actual is going on here? Shared ptrs for triangles, each triangle on the heap ........ ?????? */
		std::map<int, std::shared_ptr<Material>> meshMaterialMap;

		//Load materials.
		for (int i = 0; i < static_cast<int>(materials.size()); i++)
		{
			auto& tinyObjMaterial = materials[i];
			auto submeshMaterial = std::make_shared<Material>();
			meshMaterialMap.emplace(i, submeshMaterial);

			submeshMaterial->EmissionCoefficient = { tinyObjMaterial.emission[0], tinyObjMaterial.emission[1], tinyObjMaterial.emission[2] };
			submeshMaterial->AmbientReflectivity = { tinyObjMaterial.ambient[0], tinyObjMaterial.ambient[1], tinyObjMaterial.ambient[2] };
			submeshMaterial->AlbedoCoefficient = { tinyObjMaterial.diffuse[0], tinyObjMaterial.diffuse[1], tinyObjMaterial.diffuse[2] };
			submeshMaterial->SpecularCoefficient = { tinyObjMaterial.specular[0], tinyObjMaterial.specular[1], tinyObjMaterial.specular[2] };

			submeshMaterial->PhongExponent = static_cast<int>(tinyObjMaterial.shininess);
			submeshMaterial->RefractiveIndex = static_cast<float>(tinyObjMaterial.ior);

			if (tinyObjMaterial.dissolve < 1.0f)
			{
				// Dielectric: transparent + refractive
				submeshMaterial->IsDialetic = true;
				submeshMaterial->Reflectivity = 0.04f;  // Schlick floor, not derived from dissolve
				submeshMaterial->AbsorbanceCoefficient = { tinyObjMaterial.transmittance[0], tinyObjMaterial.transmittance[1], tinyObjMaterial.transmittance[2] };
			}
			else if (tinyObjMaterial.illum == 3 || tinyObjMaterial.illum == 4 || tinyObjMaterial.illum == 5 || tinyObjMaterial.illum == 8)
			{
				// Mirror-like reflective
				submeshMaterial->IsReflective = true;
				submeshMaterial->Reflectivity = 1.0f;
			}

			resourceManager.AddMaterial(fileName + "_" + std::to_string(i), submeshMaterial);
		}

		//Load submeshes into the model.
		for (const auto& shape : shapes)
		{
			std::map<int, std::vector<std::shared_ptr<Primitive>>> meshData;
			for (int f = 0; f < (shape.mesh.indices.size() / 3); f++)
			{
				const tinyobj::index_t verticiesIndex[3] =
				{
					shape.mesh.indices[3 * f + 0],
					shape.mesh.indices[3 * f + 1],
					shape.mesh.indices[3 * f + 2]
				};

				std::array<Vertex, 3> triangleVertices;
				for (int i = 0; i < 3; i++)
				{
					for (int compIndex = 0; compIndex < 3; compIndex++)
					{
						triangleVertices[i].Position[compIndex] = attrib.vertices[3 * verticiesIndex[i].vertex_index + compIndex];
					}

					minBounds = Min(minBounds, triangleVertices[i].Position);
					maxBounds = Max(maxBounds, triangleVertices[i].Position);
				}

				if (!attrib.normals.empty())
				{
					for (int i = 0; i < 3; i++)
					{
						for (int compIndex = 0; compIndex < 3; compIndex++)
						{
							triangleVertices[i].Normal[compIndex] = attrib.normals[3 * verticiesIndex[i].normal_index + compIndex];
						}
					}
				}
				else
				{
					/* #Note: This uses hard normals, could be smoothened to smooth lighting? */
					const glm::vec3 faceNormal = IO::CalculateNormal(triangleVertices[0].Position, triangleVertices[1].Position, triangleVertices[2].Position);
					triangleVertices[0].Normal = faceNormal;
					triangleVertices[1].Normal = faceNormal;
					triangleVertices[2].Normal = faceNormal;
				}

				int materialId = shape.mesh.material_ids[f];
				if (meshData.find(materialId) == meshData.end())
				{
					meshData.emplace(materialId, std::vector<std::shared_ptr<Primitive>>());
				}

				meshData[materialId].push_back(std::make_shared<Triangle>(triangleVertices));
			}

			//Adding the meshes to the model and adding those to the resource system.
			for (auto& data : meshData)
			{
				if (data.second.size() <= 0) continue;

				for(auto& triangle : data.second)
				{
					std::weak_ptr<Material> material = data.first == -1
						? resourceManager.GetMaterial(ResourceManager::GetDefaultMaterialName())
						: resourceManager.GetMaterial(fileName + "_" + std::to_string(data.first));

					triangles.push_back(triangle);
					triangles[triangles.size() - 1]->SetMaterial(material);
				}
			}
		}

		AABB meshBounds(minBounds, maxBounds);

		/* God I hate inheritance, this is the side product of bad architecture and shouldn't have a material that's being validated like this OR it should and the triangle shouldn't, either way I could clearly not make up my mind. */
		mesh->Initialize(resourceManager.GetMaterial(ResourceManager::GetDefaultMaterialName()), glm::vec3(0.0f, (maxBounds.y + minBounds.y) * 0.5f + abs(minBounds.y), 0.0f), glm::vec3(0.0f), glm::vec3(1.0f), meshBounds);
		mesh->SetTriangles(std::move(triangles));
		printf("Finished loading %s\n\n", filePath.c_str());
		return mesh;
	}

	glm::vec3 IO::CalculateNormal(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2)
	{
		glm::vec3 normal = glm::cross(v1 - v0, v2 - v0);
		float len2 = glm::dot(normal, normal);
		return (len2 > 0.0f) ? normal / std::sqrt(len2) : glm::vec3(0.0f);
	}
}