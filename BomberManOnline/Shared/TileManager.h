#pragma once

#include "TileLayer.h"
#include "CollidableTile.h"

class TileManager
{
	friend class Server;
	friend class Level;
public:
	TileManager();

	bool isTileOnPosition(eTileID tile, sf::Vector2f position, sf::Vector2i tileSize) const;
	bool isTileOnPosition(eTileID tile, sf::Vector2i position) const;
	bool isPositionCollidable(sf::Vector2f position, sf::Vector2i tileSize) const;
	bool isPositionCollidable(sf::Vector2i position) const;

	void removeTile(eTileID tile, sf::Vector2f position, sf::Vector2i tileSize);

private:
	std::vector<std::vector<eCollidableTile>> m_collisionLayer;
	std::vector<TileLayer> m_tileLayers;
};