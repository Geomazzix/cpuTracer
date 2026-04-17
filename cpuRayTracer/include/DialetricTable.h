#pragma once
#include <unordered_map>

namespace CRT
{
	/**
	 * @brief Used to identify the different types of refractive media that the renderer currently supports.
	 */
	enum class DialetricType : uint16_t
	{
		Air = 0,
		Glass,
		Water
	};

	/**
	 * @brief Holds the index of different refractive media - https://refractiveindex.info
	 */
	class DialetricIndexTable final
	{
	public:
		DialetricIndexTable() = default;
		~DialetricIndexTable();

		static float GetDialetricIndex(DialetricType type);

	private:
		static std::unordered_map<DialetricType, float> m_dialectricIorLookUpTable;
	};
}