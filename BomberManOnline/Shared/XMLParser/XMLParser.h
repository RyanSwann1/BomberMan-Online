#pragma once

#include <string>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

enum class eCollidableTile;
struct TileLayer;
namespace XMLParser
{
	bool parseTextureDetails(sf::Vector2i& tileSize, sf::Vector2i& textureSize, int& columns, const std::string& levelFileName, const std::string& textureFileName);
	
	bool loadMapAsClient(const std::string& mapName, sf::Vector2i& mapDimensions,
		std::vector<TileLayer>& tileLayers, std::vector<std::vector<eCollidableTile>>& collisionLayer, 
		std::vector<sf::Vector2f>& spawnPositions, std::vector<sf::Vector2f>& boxes);

	bool loadMapAsServer(const std::string& mapName, sf::Vector2i& mapDimensions,
		std::vector<std::vector<eCollidableTile>>& collisionLayer, std::vector<sf::Vector2f>& spawnPositions,
		std::vector<sf::Vector2f>& boxes);
}