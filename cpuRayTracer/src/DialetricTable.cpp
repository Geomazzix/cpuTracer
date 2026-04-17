#include "DialetricTable.h"

namespace CRT
{
	std::unordered_map<DialetricType, float> DialetricIndexTable::m_dialectricIorLookUpTable =
	{
		{ DialetricType::Air, 1.00027717f },
		{ DialetricType::Glass, 1.5168f},
		{ DialetricType::Water, 1.3325f }
	};

	DialetricIndexTable::~DialetricIndexTable()
	{
		m_dialectricIorLookUpTable.clear();
	}

	float DialetricIndexTable::GetDialetricIndex(DialetricType type)
	{
		return m_dialectricIorLookUpTable[type];
	}
}