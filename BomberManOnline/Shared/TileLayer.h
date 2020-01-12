#pragma once

#include <vector>
#include <utility>
#include <string>

struct TileLayer
{
	TileLayer(std::vector<std::vector<int>>&& tileLayer, std::string&& name)
		: m_tileLayer(std::move(tileLayer)),
		m_name(std::move(name))
	{}

	const std::vector<std::vector<int>> m_tileLayer;
	const std::string m_name;
};