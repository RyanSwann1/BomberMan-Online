#pragma once

#include "TileLayer.h"
#include <SFML/Graphics.hpp>
#include <vector>

enum class eCollidableTile;
enum class eTileID;
namespace Shared
{
	bool isTileOnPosition(const std::vector<TileLayer>& tileLayers, eTileID tileID, sf::Vector2f position, sf::Vector2i tileSize);
	bool isTileOnPosition(const std::vector<TileLayer>& tileLayers, eTileID tileID, sf::Vector2i position);
	bool isTileCollidable(const std::vector<std::vector<eCollidableTile>>& collidableLayer, sf::Vector2f position, sf::Vector2i tileSize);
	bool isTileCollidable(const std::vector<std::vector<eCollidableTile>>& collidableLayer, sf::Vector2i position);

	void changeTileAtPosition(std::vector<TileLayer>& tileLayers, std::vector<std::vector<eCollidableTile>>& collidableTileLayer, 
		eTileID newTileID, sf::Vector2f position);
}