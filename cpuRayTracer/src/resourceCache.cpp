#include "resourceCache.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "image.hpp"

namespace crt
{
	ResourceManager::ResourceManager()
	{
		std::string magentaMaterialName = std::string("No_Material_Loaded");
		std::shared_ptr<Material> material = std::make_shared<Material>();
		material->AlbedoCoefficient = glm::vec3(1.0f, 0.0f, 1.0f);
		material->SpecularCoefficient = glm::vec3(0.5f);
		material->PhongExponent = 1024;
		material->IsReflective = false;
		m_materials.emplace(magentaMaterialName, material);
	}

	ResourceManager::~ResourceManager()
	{
		m_materials.clear();
	}

	std::string ResourceManager::GetDefaultMaterialName()
	{
		return "No_Material_Loaded";
	}

	void ResourceManager::AddMaterial(const std::string& name, std::shared_ptr<Material> material)
	{
		if (m_materials.find(name) != m_materials.end())
		{
			printf("WARNING: The material '%s' has already been stored and will therefor not be stored again!\n", name.c_str());
			return;
		}
		m_materials.emplace(name, material);
	}

	void ResourceManager::DeleteMaterial(const std::string& name)
	{
		if (m_materials.find(name) == m_materials.end())
		{
			printf("WARNING: The material '%s' has not yet been stored before and will therefor not be deleted!\n", name.c_str());
			return;
		}
		m_materials.erase(name);
	}

	std::weak_ptr<Material> ResourceManager::GetMaterial(const std::string& name)
	{
		return m_materials[name];
	}
}