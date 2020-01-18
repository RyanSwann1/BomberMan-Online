#pragma once

#include <vector>
#include <utility>
#include <string>
#include <SFML/Graphics.hpp>

enum class eTileID;
struct TileLayer
{
	TileLayer(std::vector<std::vector<int>>&& tileLayer, std::string&& name);

	int getTileID(sf::Vector2i position) const;
	void changeTile(eTileID newTile, sf::Vector2i position);
	void removeTile(eTileID tileToRemove, sf::Vector2i position);
	void changeTile(eTileID newTileID, sf::Vector2i position);

	std::vector<std::vector<int>> m_tileLayer;
	const std::string m_name;
};