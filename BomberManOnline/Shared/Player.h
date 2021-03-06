#pragma once

#include <SFML/Graphics.hpp>
#include "PlayerControllerType.h"
#include "Timer.h"
#include "NonCopyable.h"
#include "Direction.h"

constexpr float MOVEMENT_SPEED_INCREMENT = 0.2f;
constexpr int INVALID_PLAYER_ID = -1;

class Player : private NonCopyable
{
public:
	Player(int ID, sf::Vector2f startingPosition, ePlayerControllerType controllerType);
	virtual ~Player() {}

	eDirection getFacingDirection() const;
	bool isMoving() const;
	ePlayerControllerType getControllerType() const;
	int getID() const;
	int getCurrentBombExplosionSize() const;
	sf::Vector2f getPosition() const;
	sf::Vector2f getNewPosition() const;
	sf::Vector2f getPreviousPosition() const;

	virtual bool placeBomb();
	virtual void update(float deltaTime);
	void increaseMovementSpeed(float amount);
	void increaseBombCount();
	void increaseBombExplosionSize();

protected:
	const int m_maxBombCount;
	const int m_maxBombExplosionSize;
	int m_currentBombCount;
	int m_bombsPlaced;
	int m_currentBombExplosionSize;
	int m_ID;
	sf::Vector2f m_previousPosition;
	sf::Vector2f m_position;
	sf::Vector2f m_newPosition;
	ePlayerControllerType m_controllerType;
	eDirection m_facingDirection;
	float m_movementFactor;
	float m_movementSpeed;
	Timer m_bombPlacementTimer;
};