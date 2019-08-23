#pragma once

#include "NonCopyable.h"
#include "TileLayer.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>

class Level : private NonCopyable
{
public:
	static std::unique_ptr<Level> create(const std::string& levelName);

	const std::vector<sf::Vector2i>& getCollisionLayer() const;

	void render(sf::RenderWindow& window) const;

private:
	Level() {}
	std::string m_levelName;
	sf::Vector2i m_levelDimensions;
	std::vector<sf::Vector2i> m_collisionLayer;
	std::vector<TileLayer> m_tileLayers;
	std::vector<sf::Vector2i> m_spawnPositions;
};