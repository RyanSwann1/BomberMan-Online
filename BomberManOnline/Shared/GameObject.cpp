#include "GameObject.h"
#include "Utilities.h"

GameObject::GameObject(sf::Vector2f startingPosition, float expirationTime, eGameObjectType type, eTimerActive timerActive)
	: m_type(type),
	m_movementSpeed(2.5f),
	m_movementFactor(0.0f),
	m_newPosition(startingPosition),
	m_previousPosition(startingPosition),
	m_position(startingPosition),
	m_lifeTimer(expirationTime, timerActive)
{}

bool GameObject::isPickUp() const
{
	return (m_type == eGameObjectType::eMovementPickUp ||
		m_type == eGameObjectType::eExtraBombPickUp ||
		m_type == eGameObjectType::eBiggerExplosionPickUp);
}

bool GameObject::isMoving() const
{
	return m_position != m_newPosition;
}

eGameObjectType GameObject::getType() const
{
	return m_type;
}

const Timer & GameObject::getTimer() const
{
	return m_lifeTimer;
}

sf::Vector2f GameObject::getPosition() const
{
	return m_position;
}

sf::Vector2f GameObject::getNewPosition() const
{
	return m_newPosition;
}

void GameObject::update(float frameTime)
{
	m_lifeTimer.update(frameTime);

	if (isMoving())
	{
		m_movementFactor += frameTime * m_movementSpeed;
		m_position = Utilities::Interpolate(m_previousPosition, m_newPosition, m_movementFactor);
	}
}

void GameObject::setNewPosition(sf::Vector2f newPosition)
{
	if (!isMoving())
	{
		m_newPosition = newPosition;
	}
}