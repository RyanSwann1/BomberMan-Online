#include "Player.h"
#include "Utilities.h"

constexpr int MAX_BOMB_COUNT = 5;

Player::Player(int ID, sf::Vector2f startingPosition, ePlayerControllerType controllerType)
	: m_maxBombCount(MAX_BOMB_COUNT),
	m_currentBombCount(1),
	m_ID(ID),
	m_previousPosition(),
	m_position(startingPosition),
	m_newPosition(startingPosition),
	m_controllerType(controllerType),
	m_moveDirection(),
	m_movementFactor(0.0f),
	m_movementSpeed(2.5f),
	m_bombPlacementTimer(2.0f, eTimerActive::eTrue)
{}

Timer & Player::getBombPlacementTimer()
{
	return m_bombPlacementTimer;
}

bool Player::isMoving() const
{
	return m_position != m_newPosition;
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

bool Player::placeBomb()
{
	if (!isMoving())
	{
		if (m_bombsPlaced == 0)
		{
			m_bombPlacementTimer.setActive(true);
		}

		if (m_bombsPlaced < m_currentBombCount)
		{
			++m_bombsPlaced;
			return true;
		}
	}

	return false;
}

void Player::update(float deltaTime)
{
	m_bombPlacementTimer.update(deltaTime);
	if (m_bombPlacementTimer.isExpired())
	{
		m_bombsPlaced = 0;
		m_bombPlacementTimer.resetElaspedTime();
		m_bombPlacementTimer.setActive(false);
	}

	if (isMoving())
	{
		m_movementFactor += deltaTime * m_movementSpeed;
		m_position = Utilities::Interpolate(m_previousPosition, m_newPosition, m_movementFactor);

		if (m_position == m_newPosition)
		{
			m_movementFactor = 0.0f;
		}
	}
}

void Player::stop()
{
	m_movementFactor = 0.0f;
}

void Player::increaseMovementSpeed(float amount)
{
	m_movementSpeed += amount;
}

void Player::increaseBombCount()
{
	if (m_currentBombCount < m_maxBombCount)
	{
		++m_currentBombCount;
	}
}