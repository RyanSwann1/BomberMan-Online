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

	eGameObjectType getType() const;
	const Timer& getTimer() const;
	sf::Vector2f getPosition() const;

	void update(float frameTime);

private:
	eGameObjectType m_type;
	sf::Vector2f m_position;
	Timer m_lifeTimer;
};