#include "BombServer.h"
#include "Utilities.h"

BombServer::BombServer(sf::Vector2f startingPosition, int explosionSize)
	: GameObject(startingPosition, BOMB_LIFETIME_DURATION, eGameObjectType::eBomb, eTimerActive::eTrue),
	m_explosionSize(explosionSize)
{}

bool BombServer::isMoving() const
{
	return m_position != m_newPosition;
}

int BombServer::getExplosionSize() const
{
	return m_explosionSize;
}

void BombServer::update(float deltaTime)
{
	if (isMoving())
	{
		m_movementFactor += deltaTime * m_movementSpeed;
		m_position = Utilities::Interpolate(m_previousPosition, m_newPosition, m_movementFactor);	
	}
}