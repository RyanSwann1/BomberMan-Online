#pragma once

#include "PlayerControllerType.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

struct PlayerDetails
{
	PlayerDetails(int ID, sf::Vector2f spawnPosition, ePlayerControllerType controllerType)
		: ID(ID),
		spawnPosition(spawnPosition),
		controllerType(controllerType)
	{}

	int ID;
	sf::Vector2f spawnPosition;
	ePlayerControllerType controllerType;
};

struct ServerMessageInitialGameData
{
	std::string levelName;
	std::vector<PlayerDetails> playerDetails;
};

struct ServerMessageInvalidMove
{
	ServerMessageInvalidMove()
		: invalidPosition(),
		lastValidPosition()
	{}

	ServerMessageInvalidMove(sf::Vector2f invalidPosition, sf::Vector2f lastValidPosition)
		: invalidPosition(invalidPosition),
		lastValidPosition(lastValidPosition)
	{}

	sf::Vector2f invalidPosition;
	sf::Vector2f lastValidPosition;
};

struct ServerMessagePlayerMove
{
	ServerMessagePlayerMove()
		: newPosition(),
		speed(0)
	{}

	ServerMessagePlayerMove(sf::Vector2f newPosition, float speed)
		: newPosition(newPosition),
		speed(speed)
	{}

	sf::Vector2f newPosition;
	float speed;
};

struct ServerMessageBombPlacement
{
	ServerMessageBombPlacement(sf::Vector2f position, int playerID)
		: position(position),
		playerID(playerID)
	{}

	sf::Vector2f position;
	int playerID;
};