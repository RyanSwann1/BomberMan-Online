#pragma once

#include "TileLayer.h"
#include "CollidableTile.h"
#include "NonCopyable.h"

class Texture;
class TileManager : private NonCopyable
{
	friend class Server;
	friend class Level;
public:
	TileManager();

	const std::vector<std::vector<eCollidableTile>>& getCollisionLayer() const;
	bool isTileOnPosition(eTileID tile, sf::Vector2i position) const;
	bool isPositionCollidable(sf::Vector2i position) const;
	bool isPositionAdjacentToBox(sf::Vector2i position) const;

	void removeTile(eTileID tileToRemove, sf::Vector2i position);
	void render(sf::RenderWindow& window, sf::Vector2i levelSize) const;

private:
	std::vector<std::vector<eCollidableTile>> m_collisionLayer;
	std::vector<TileLayer> m_tileLayers;

	eTileID getTileAtPosition(const std::string& tileLayerName, sf::Vector2i position) const;
	TileLayer& getTileLayer(const std::string& name);
	const TileLayer& getTileLayer(const std::string& name) const;
};