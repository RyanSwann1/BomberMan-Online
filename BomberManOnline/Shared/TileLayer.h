#pragma once

#include "TileID.h"
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>

struct TileLayer
{
	TileLayer(std::vector<std::vector<int>>&& tileLayer, std::string&& name);

	eTileID getTile(sf::Vector2i position) const;
	void changeTile(sf::Vector2i position, eTileID newTileID);

	std::vector<std::vector<int>> m_tileLayer;
	const std::string m_name;
};