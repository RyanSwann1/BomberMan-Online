#pragma once

#include "PlayerControllerType.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

struct PlayerDetails
{
	PlayerDetails(int ID, sf::Vector2i spawnPosition, ePlayerControllerType controllerType)
		: ID(ID),
		spawnPosition(spawnPosition),
		controllerType(controllerType)
	{}

	int ID;
	sf::Vector2i spawnPosition;
	ePlayerControllerType controllerType;
};

struct ServerMessageInitialGameData
{
	std::string levelName;
	std::vector<PlayerDetails> playerDetails;
};

struct ServerMessageInvalidMove
{
	sf::Vector2f invalidPosition;
	sf::Vector2f lastValidPosition;
};