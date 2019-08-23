#pragma once

#include "NonCopyable.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>

struct TileLayer
{
	TileLayer(std::vector<std::vector<int>>&& tileLayer);

	void render(sf::RenderWindow& window, sf::Vector2i levelDimensions) const;

	const std::vector<std::vector<int>> m_tileLayer;
};

class Level : private NonCopyable
{
public:
	static std::unique_ptr<Level> create(const std::string& levelName);

	void render(sf::RenderWindow& window);

private:
	Level() {}
	std::string m_levelName;
	sf::Vector2i m_levelDimensions;
	std::vector<sf::Vector2i> m_collisionLayer;
	std::vector<TileLayer> m_tileLayers;
	std::vector<sf::Vector2i> m_spawnPositions;
	//Bomb
	//Players
	//Pick ups
};