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

int Player::getID() const
{
	return m_ID;
}

sf::Vector2f Player::getCurrentPosition() const
{
	return m_position;
}

sf::Vector2f Player::getNewPosition() const
{
	return m_newPosition;
}

void Player::update(float deltaTime)
{
	if (m_moving)
	{
		m_movementFactor += deltaTime * m_movementSpeed;
		m_position = Utilities::Interpolate(m_previousPosition, m_newPosition, m_movementFactor);
	}

	m_bombPlacementTimer.update(deltaTime);
}

void Player::stop()
{
	m_moving = false;
	m_movementFactor = 0;
}