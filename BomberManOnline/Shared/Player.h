#pragma once

#include <SFML/Graphics.hpp>
#include "PlayerControllerType.h"
#include "Timer.h"
#include "Direction.h"

constexpr float MOVEMENT_SPEED_INCREMENT = 0.2f;

class Player
{
public:
	Player(int ID, sf::Vector2f startingPosition, ePlayerControllerType controllerType);
	virtual ~Player() {}

	Timer& getBombPlacementTimer();
	bool isMoving() const;
	ePlayerControllerType getControllerType() const;
	int getID() const;
	sf::Vector2f getPosition() const;
	sf::Vector2f getNewPosition() const;
	sf::Vector2f getPreviousPosition() const;

	virtual void update(float deltaTime);
	void stop();
	void increaseMovementSpeed(float amount);
	virtual void setNewPosition(sf::Vector2f newPosition);

protected:
	int m_ID;
	sf::Vector2f m_previousPosition;
	sf::Vector2f m_position;
	sf::Vector2f m_newPosition;
	ePlayerControllerType m_controllerType;
	eDirection m_moveDirection;
	bool m_moving;
	float m_movementFactor;
	float m_movementSpeed;
	Timer m_bombPlacementTimer;
};