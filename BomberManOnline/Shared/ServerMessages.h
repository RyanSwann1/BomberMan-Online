#pragma once

#include "PlayerControllerType.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

struct PlayerDetails
{
	int ID;
	sf::Vector2i spawnPosition;
	ePlayerControllerType controllerType;
};

struct ServerMessageInitialGameData
{


	std::string levelName;
	std::vector<PlayerDetails> playerSpawnPositions;
};