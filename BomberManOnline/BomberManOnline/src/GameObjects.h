#pragma once

#include <SFML/Graphics.hpp>
#include "Direction.h"
#include "Timer.h"

struct Player
{
	Player(int tileSize, int ID);
	Player(int tileSize, int ID, sf::Vector2f startingPosition);

	void setNewPosition(sf::Vector2f newPosition);
	void plantBomb();

	int m_ID;
	sf::Vector2f m_position;
	sf::Vector2f m_previousPosition;
	sf::Vector2f m_newPosition;
	eDirection m_movementDirection;
	float m_movementSpeed;
	float m_movementFactor;
	sf::RectangleShape m_shape;
	sf::FloatRect m_AABB;
	bool m_moving;
	Timer m_bombPlacementTimer;
};

struct Bomb
{
	Bomb(sf::Vector2f startingPosition, float expirationTime);

	sf::Vector2f m_position;
	sf::Sprite m_sprite;
	Timer m_lifeTimer;
};