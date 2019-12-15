#include "Player.h"
#include "Utilities.h"

Player::Player(int ID, sf::Vector2f startingPosition, ePlayerControllerType controllerType)
	: m_maxBombCount(5),
	m_maxBombExplosionSize(5),
	m_currentBombCount(1),
	m_bombsPlaced(0),
	m_currentBombExplosionSize(1),
	m_ID(ID),
	m_previousPosition(),
	m_position(startingPosition),
	m_newPosition(startingPosition),
	m_controllerType(controllerType),
	m_facingDirection(),
	m_movementFactor(0.0f),
	m_movementSpeed(2.5f),
	m_bombPlacementTimer(2.0f, eTimerActive::eTrue)
{}

bool Player::isBombCountReached() const
{
	return m_currentBombCount < m_maxBombCount;
}

eDirection Player::getFacingDirection() const
{
	return m_facingDirection;
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

int Player::getCurrentBombExplosionSize() const
{
	return m_currentBombExplosionSize;
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
	if (m_bombsPlaced == 0)
	{
		m_bombPlacementTimer.setActive(true);
	}

	if (m_bombsPlaced < m_currentBombCount)
	{
		++m_bombsPlaced;
		return true;
	}
	else
	{
		return false;
	}
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

		if (!isMoving())
		{
			m_movementFactor = 0.0f;
		}
	}
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

void Player::increaseBombExplosionSize()
{
	if (m_currentBombExplosionSize < m_maxBombExplosionSize)
	{
		++m_currentBombExplosionSize;
	}
}
