#pragma once

#include "Player.h"

struct PlayerClient : public Player
{
	PlayerClient(int tileSize, int ID, sf::Vector2f startingPosition);

	void setNewPosition(sf::Vector2f newPosition);
	void plantBomb();

	sf::RectangleShape m_shape;
	sf::FloatRect m_AABB;
};

struct Bomb
{
	Bomb(sf::Vector2f startingPosition, float expirationTime);

	sf::Vector2f m_position;
	sf::Sprite m_sprite;
	Timer m_lifeTimer;
};

struct Explosion
{
	Explosion(sf::Vector2f startingPosition, float expirationTime);

	void update(float deltaTime);

	sf::Vector2f m_position;
	sf::Sprite m_sprite;
	Timer m_lifeTimer;
};