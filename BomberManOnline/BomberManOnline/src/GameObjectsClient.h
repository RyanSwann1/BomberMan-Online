#pragma once

#include "Player.h"
#include "Direction.h"
#include "AnimatedSprite.h"
#include "GameObject.h"
#include <vector>

constexpr float EXPLOSION_LIFETIME_DURATION = 0.5f;
constexpr size_t MAX_PREVIOUS_POINTS = 10;

enum class eCollidableTile;
struct MovementPoint;
class PlayerClient : public Player
{
public:
	PlayerClient(int ID, sf::Vector2f startingPosition);

	void update(float deltaTime) override;
	void render(sf::RenderWindow& window) const;

	void setRemotePlayerPosition(sf::Vector2f newPosition);
	void setLocalPlayerPosition(sf::Vector2f newPosition, const std::vector<std::vector<eCollidableTile>>& collisionLayer, sf::Vector2i tileSize,
		std::vector<MovementPoint>& localPlayerPreviousPositions);

	void plantBomb();
	void stopAtPosition(sf::Vector2f position);

private:
	AnimatedSprite m_sprite;
	eDirection m_moveDirection;
};

class GameObjectClient : public GameObject
{
public:
	GameObjectClient(sf::Vector2f startingPosition, float expirationTime, eAnimationName startingAnimationName,
		eGameObjectType type, eGameObjectTag tag = eGameObjectTag::eNone, eTimerActive timerActive = eTimerActive::eFalse);

	void render(sf::RenderWindow& window) const;
	void update(float deltaTime);
	
private:
	AnimatedSprite m_sprite;
};