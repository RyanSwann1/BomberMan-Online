#include "GameObject.h"

GameObject::GameObject(sf::Vector2f startingPosition, float expirationTime, eGameObjectType type, eTimerActive timerActive)
	: m_type(type),
	m_position(startingPosition),
	m_lifeTimer(expirationTime, timerActive)
{}

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