#pragma once

#include "Timer.h"
#include <SFML/Graphics.hpp>

constexpr float BOMB_LIFETIME_DURATION = 2.0f;

enum class eGameObjectType
{
	eBomb = 0,
	eExplosion,
	eMovementPickUp,
	eExtraBombPickUp,
	eBiggerExplosionPickUp
};

class GameObject
{
public:
	GameObject(sf::Vector2f startingPosition, float expirationTime, eGameObjectType type, eTimerActive timerActive = eTimerActive::eTrue);

	bool isMoving() const;
	eGameObjectType getType() const;
	const Timer& getTimer() const;
	sf::Vector2f getPosition() const;

	virtual void update(float frameTime);
	void setNewPosition(sf::Vector2f newPosition);

protected:
	eGameObjectType m_type;
	float m_movementSpeed;
	float m_movementFactor;
	sf::Vector2f m_previousPosition;
	sf::Vector2f m_position;
	sf::Vector2f m_newPosition;
	Timer m_lifeTimer;
};