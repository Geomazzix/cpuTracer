#pragma once
#include <memory>
#include <string>
#include <unordered_map>

namespace crt
{
	struct Material;
	struct Image;

	/// <summary>
	/// The resource manager stores instances of data, which can be reused.
	/// </summary>
	class ResourceManager final
	{
	public:
		ResourceManager();
		~ResourceManager();

		static std::string GetDefaultMaterialName();

		void AddMaterial(const std::string& name, std::shared_ptr<Material> material);
		void DeleteMaterial(const std::string& name);
		std::weak_ptr<Material> GetMaterial(const std::string& name);

	private:
		std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;
	};
}