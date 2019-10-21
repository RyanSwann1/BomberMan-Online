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

struct GameObject
{
	GameObject(sf::Vector2f startingPosition, float expirationTime, eGameObjectType type, eGameObjectTag tag = eGameObjectTag::eNone)
		: m_type(type),
		m_position(startingPosition),
		m_lifeTimer(expirationTime),
		m_tag(tag)
	{}

	eGameObjectType m_type;
	sf::Vector2f m_position;
	Timer m_lifeTimer;
	eGameObjectTag m_tag;
};