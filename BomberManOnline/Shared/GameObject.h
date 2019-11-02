#pragma once

#include "Timer.h"
#include <SFML/Graphics.hpp>

enum class eGameObjectType
{
	eBomb = 0,
	eExplosion,
	eMovementPickUp
};

enum class eGameObjectTag
{
	eNone = 0,
	ePickUp
};

class GameObject
{
public:
	GameObject(sf::Vector2f startingPosition, float expirationTime, eGameObjectType type, eGameObjectTag tag = eGameObjectTag::eNone);

	eGameObjectType getType() const;
	eGameObjectTag getTag() const;
	const Timer& getTimer() const;
	sf::Vector2f getPosition() const;

	void update(float frameTime);

private:
	eGameObjectType m_type;
	sf::Vector2f m_position;
	Timer m_lifeTimer;
	eGameObjectTag m_tag;
};