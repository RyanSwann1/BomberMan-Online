#pragma once

#include "AnimatedSprite.h"
#include "GameObject.h"
#include <vector>

constexpr float EXPLOSION_LIFETIME_DURATION = 0.5f;

class GameObjectClient : public GameObject
{
public:
	GameObjectClient(sf::Vector2f startingPosition, float expirationTime, eAnimationName startingAnimationName,
		eGameObjectType type, eTimerActive timerActive = eTimerActive::eFalse);

	void render(sf::RenderWindow& window) const;
	void update(float deltaTime);
	
private:
	AnimatedSprite m_sprite;
};

class BombClient : public GameObjectClient
{
public:
	BombClient(sf::Vector2f startingPosition, int explosionRange);

	int getExplosionSize() const;

private:
	int m_explosionSize;
};