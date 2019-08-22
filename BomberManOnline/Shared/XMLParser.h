#pragma once

#include <string>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

struct FrameDetails;
class Texture;
namespace XMLParser
{
	bool loadTextureDetails(const std::string& fileName, std::string& imagePath, std::vector<FrameDetails>& frames);
	
	bool loadMapAsClient(const std::string& mapName, sf::Vector2i& mapDimensions,
		std::vector<std::vector<int>>& tileData, std::vector<sf::Vector2i>& collisionLayer, std::vector<sf::Vector2i>& spawnPositions);

	bool loadMapAsServer(const std::string& mapName, sf::Vector2i& mapDimensions,
		std::vector<sf::Vector2i>& collisionLayer, std::vector<sf::Vector2i>& spawnPositions);
}