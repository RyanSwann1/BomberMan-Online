#pragma once

#include "PlayerControllerType.h"
#include <SFML/Graphics.hpp>
#include "Direction.h"
#include <vector>
#include <string>

struct PlayerDetails
{
	PlayerDetails(int ID, sf::Vector2f spawnPosition)
		: ID(ID),
		spawnPosition(spawnPosition)
	{}

	int ID;
	sf::Vector2f spawnPosition;
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
	ServerMessageBombPlacement()
		: position(),
		lifeTimeDuration(0),
		explosionSize(0)
	{}

	ServerMessageBombPlacement(sf::Vector2f position, float lifeTimeDuration, int explosionSize)
		: position(position),
		lifeTimeDuration(lifeTimeDuration),
		explosionSize(explosionSize)
	{}

	sf::Vector2f position;
	float lifeTimeDuration;
	int explosionSize;
};

struct ServerMessagePickUpCollision
{
	ServerMessagePickUpCollision()
	{

	}


};