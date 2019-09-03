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
	
	bool loadLevelAsClient(const std::string& levelName, sf::Vector2i& levelSize,
		std::vector<TileLayer>& tileLayers, std::vector<std::vector<eCollidableTile>>& collisionLayer, 
		std::vector<sf::Vector2f>& spawnPositions, sf::Vector2i tileSize);

	bool loadLevelAsServer(const std::string& levelName, sf::Vector2i& levelSize,
		std::vector<std::vector<eCollidableTile>>& collisionLayer, std::vector<sf::Vector2f>& spawnPositions, sf::Vector2i tileSize);
}