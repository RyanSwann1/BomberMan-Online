#pragma once

#include "Player.h"
#include "Direction.h"
#include "AnimatedSprite.h"
#include "GameObjectType.h"
#include <vector>

constexpr float BOMB_LIFETIME_DURATION = 2.0f;
constexpr float EXPLOSION_LIFETIME_DURATION = 0.5f;
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
	PlayerClient(int ID, sf::Vector2f startingPosition);

	virtual void setNewPosition(sf::Vector2f newPosition);
	void plantBomb();

	AnimatedSprite m_sprite;
	eDirection m_moveDirection;
};

struct PlayerClientLocalPlayer : public PlayerClient
{
	PlayerClientLocalPlayer(int ID, sf::Vector2f startingPosition);

	void setNewPosition(sf::Vector2f newPosition) override final;

	std::vector<MovementPoint> m_previousPositions;
};

//Bomb, Explosion
struct GameObjectClient
{
	GameObjectClient(sf::Vector2f startingPosition, float expirationTime, eAnimationName startingAnimationName, eGameObjectType type);

	eGameObjectType m_type;
	sf::Vector2f m_position;
	AnimatedSprite m_sprite;
	Timer m_lifeTimer;
};

struct PickUpClient
{
	PickUpClient(sf::Vector2f startingPosition, sf::Color color, sf::Vector2i tileSize, eGameObjectType pickUpType)
		: m_position(startingPosition),
		m_shape(sf::Vector2f(tileSize.x, tileSize.y)),
		m_type(pickUpType)
	{
		m_shape.setFillColor(color);
		m_shape.setPosition(startingPosition);
	}

	sf::Vector2f m_position;
	sf::RectangleShape m_shape;
	eGameObjectType m_type;
};