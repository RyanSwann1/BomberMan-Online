#pragma once

#include "GameObject.h"

class BombServer : public GameObject
{
public:
	BombServer(sf::Vector2f startingPosition, int explosionSize);

	bool isMoving() const;
	int getExplosionSize() const;

	virtual void update(float deltaTime) override;

private:
	int m_explosionSize;
};