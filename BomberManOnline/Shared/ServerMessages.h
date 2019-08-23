#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

struct PlayerSpawnPosition
{
	int ID;
	sf::Vector2i spawnPosition;
};

struct ServerMessageInitialGameData
{
	std::string levelName;
	std::vector<PlayerSpawnPosition> playerSpawnPositions;
};