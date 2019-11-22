#pragma once

#include "GameObject.h"

class BombServer : public GameObject
{
public:
	BombServer(sf::Vector2f startingPosition, int explosionSize)
		: GameObject(startingPosition, BOMB_LIFETIME_DURATION, eGameObjectType::eBomb, eTimerActive::eTrue),
		m_explosionSize(explosionSize)
	{}

	int getExplosionSize() const
	{
		return m_explosionSize;
	}

private:
	int m_explosionSize;
};