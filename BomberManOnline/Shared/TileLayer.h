#pragma once

#include <vector>
#include <utility>

struct TileLayer
{
	TileLayer(std::vector<std::vector<int>>&& tileLayer)
		: m_tileLayer(std::move(tileLayer))
	{}

	const std::vector<std::vector<int>> m_tileLayer;
};