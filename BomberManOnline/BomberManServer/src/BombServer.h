#pragma once

#include "GameObject.h"

class BombServer : public GameObject
{
public:
	BombServer(sf::Vector2f startingPosition, int explosionSize);

	int getExplosionSize() const;

private:
	int m_explosionSize;
};