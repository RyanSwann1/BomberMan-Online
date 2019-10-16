#pragma once

#include "Player.h"
#include "Direction.h"
#include "AnimatedSprite.h"
#include "GameObjectType.h"
#include <vector>

constexpr float BOMB_LIFETIME_DURATION = 2.0f;
constexpr float EXPLOSION_LIFETIME_DURATION = 0.5f;
constexpr size_t MAX_PREVIOUS_POINTS = 10;

enum class eCollidableTile;
struct MovementPoint;
struct PlayerClient : public Player
{
	PlayerClient(int ID, sf::Vector2f startingPosition);

	void setRemotePlayerPosition(sf::Vector2f newPosition);
	void setLocalPlayerPosition(sf::Vector2f newPosition, const std::vector<std::vector<eCollidableTile>>& collisionLayer, sf::Vector2i tileSize,
		std::vector<MovementPoint>& localPlayerPreviousPositions);
	void plantBomb();

	AnimatedSprite m_sprite;
	eDirection m_moveDirection;
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