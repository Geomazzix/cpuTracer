#pragma once
#include <vec3.hpp>
#include <memory>
#include <string>

namespace CRT
{
	struct Image;
	class Mesh;
	class ResourceManager;
	
	/**
	 * @brief The IO class contains a set of helper functions for reading/writing from/to files.
	 */
	class IO final
	{
	public:
		static std::string GetNameFromFilePath(const std::string& filePath);
		static void StorePNG(const char* filePath, unsigned int width, unsigned int height, unsigned int numChannels, glm::vec3* colorData, bool normalizedColorData = false);

		static std::shared_ptr<Image> LoadImage(const char* filePath);
		static std::shared_ptr<Mesh> LoadWavefrontFile(ResourceManager& resourceManager, const std::string& filePath);

	private:
		static glm::vec3 CalculateNormal(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2);
	};
}