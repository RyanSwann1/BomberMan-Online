#pragma once

#include "Player.h"
#include "Direction.h"
#include <vector>

constexpr size_t MAX_PREVIOUS_POINTS = 10;

struct MovementPoint
{
	MovementPoint(sf::Vector2f position, eDirection moveDirection)
		: position(position),
		moveDirection(moveDirection)
	{}

	sf::Vector2f position;
	eDirection moveDirection;
};

struct PlayerClient : public Player
{
	PlayerClient(int tileSize, int ID, sf::Vector2f startingPosition);

	virtual void setNewPosition(sf::Vector2f newPosition);
	void plantBomb();

	sf::RectangleShape m_shape;
	sf::FloatRect m_AABB;
	eDirection m_moveDirection;
};

struct PlayerClientLocalPlayer : public PlayerClient
{
	PlayerClientLocalPlayer(int tileSize, int ID, sf::Vector2f startingPosition)
		: PlayerClient(tileSize, ID, startingPosition)
	{}

	void setNewPosition(sf::Vector2f newPosition) override final;

	std::vector<MovementPoint> m_previousPositions;
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