#include "GameObject.h"

GameObject::GameObject(sf::Vector2f startingPosition, float expirationTime, eGameObjectType type, eTimerActive timerActive)
	: m_type(type),
	m_movementSpeed(5.0f),
	m_movementFactor(0.0f),
	m_newPosition(),
	m_previousPosition(),
	m_position(startingPosition),
	m_lifeTimer(expirationTime, timerActive)
{}

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

void GameObject::update(float frameTime)
{
	m_lifeTimer.update(frameTime);
}

void GameObject::setNewPosition(sf::Vector2f newPosition)
{
	if (!isMoving())
	{
		m_newPosition = newPosition;
	}
}