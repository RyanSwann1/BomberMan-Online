#pragma once

#include "Player.h"
#include "Direction.h"
#include "AnimatedSprite.h"
#include "GameObject.h"
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

struct GameObjectClient : public GameObject
{
	GameObjectClient(sf::Vector2f startingPosition, float expirationTime, eAnimationName startingAnimationName,
		eGameObjectType type, eGameObjectTag tag = eGameObjectTag::eNone);

	void update(float deltaTime);

	AnimatedSprite m_sprite;
};