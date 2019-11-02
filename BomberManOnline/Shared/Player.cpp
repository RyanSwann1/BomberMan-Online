#include "Player.h"
#include "Utilities.h"

Player::Player(int ID, sf::Vector2f startingPosition, ePlayerControllerType controllerType)
	: m_ID(ID),
	m_previousPosition(),
	m_position(startingPosition),
	m_newPosition(),
	m_controllerType(controllerType),
	m_moveDirection(),
	m_moving(false),
	m_movementFactor(0.0f),
	m_movementSpeed(2.5f),
	m_bombPlacementTimer(2.0f, true)
{}

Timer & Player::getBombPlacementTimer()
{
	return m_bombPlacementTimer;
}

bool Player::isMoving() const
{
	return m_moving;
}

ePlayerControllerType Player::getControllerType() const
{
	return m_controllerType;
}

int Player::getID() const
{
	return m_ID;
}

sf::Vector2f Player::getPosition() const
{
	return m_position;
}

sf::Vector2f Player::getNewPosition() const
{
	return m_newPosition;
}

sf::Vector2f Player::getPreviousPosition() const
{
	return m_previousPosition;
}

void Player::update(float deltaTime)
{
	m_bombPlacementTimer.update(deltaTime);

	if (m_moving)
	{
		m_movementFactor += deltaTime * m_movementSpeed;
		m_position = Utilities::Interpolate(m_previousPosition, m_newPosition, m_movementFactor);

		if (m_position == m_newPosition)
		{
			m_moving = false;
			m_movementFactor = 0;
		}
	}
}

void Player::stop()
{
	m_moving = false;
	m_movementFactor = 0;
}

void Player::increaseMovementSpeed(float amount)
{
	m_movementSpeed += amount;
}

void Player::setNewPosition(sf::Vector2f newPosition)
{
	m_newPosition = newPosition;
	m_previousPosition = m_position;
	m_moving = true;
}