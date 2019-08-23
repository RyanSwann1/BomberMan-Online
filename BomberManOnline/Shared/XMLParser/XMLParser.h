#pragma once

#include <string>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

struct TileLayer;
namespace XMLParser
{
	bool parseTextureDetails(sf::Vector2i& tileSize, sf::Vector2i& textureSize, int& columns, const std::string& levelFileName, const std::string& textureFileName);
	
	bool loadMapAsClient(const std::string& mapName, sf::Vector2i& mapDimensions,
		std::vector<TileLayer>& tileLayers, std::vector<sf::Vector2i>& collisionLayer, std::vector<sf::Vector2i>& spawnPositions);

	bool loadMapAsServer(const std::string& mapName, sf::Vector2i& mapDimensions,
		std::vector<sf::Vector2i>& collisionLayer, std::vector<sf::Vector2i>& spawnPositions);
}