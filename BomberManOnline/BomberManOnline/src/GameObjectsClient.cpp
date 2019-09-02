#include "GameObjectsClient.h"
#include "NetworkHandler.h"
#include "ServerMessageType.h"
#include "ServerMessages.h"
#include "Resources.h"

//Player
PlayerClient::PlayerClient(int tileSize, int ID, sf::Vector2f startingPosition)
	: Player(ID, startingPosition, ePlayerControllerType::eHuman),
	m_sprite(startingPosition, eAnimationName::ePlayerIdleDown),
	m_AABB(startingPosition, sf::Vector2f(static_cast<float>(tileSize), static_cast<float>(tileSize)))
{}

void PlayerClient::setNewPosition(sf::Vector2f newPosition)
{
	m_newPosition = newPosition;
	m_previousPosition = m_position;
	m_moving = true;

	//Assign new movement direction
	if (newPosition.x > m_position.x)
	{
		m_moveDirection = eDirection::eRight;
		m_sprite.setNewAnimation(eAnimationName::ePlayerMoveRight);
	}
	else if (newPosition.x < m_position.x)
	{
		m_moveDirection = eDirection::eLeft;
		m_sprite.setNewAnimation(eAnimationName::ePlayerMoveLeft);
	}
	else if (newPosition.y < m_position.y)
	{
		m_moveDirection = eDirection::eUp;
		m_sprite.setNewAnimation(eAnimationName::ePlayerMoveUp);
	}
	else if (newPosition.y > m_position.y)
	{
		m_moveDirection = eDirection::eDown;
		m_sprite.setNewAnimation(eAnimationName::ePlayerMoveDown);
	}
}

void PlayerClient::plantBomb()
{
	if (!m_moving && m_bombPlacementTimer.isExpired())
	{
		m_bombPlacementTimer.resetElaspedTime();

		sf::Packet packetToSend;
		packetToSend << eServerMessageType::ePlayerBombPlacementRequest << m_position.x << m_position.y;
		NetworkHandler::getInstance().sendMessageToServer(packetToSend);
	}
}

void PlayerClientLocalPlayer::setNewPosition(sf::Vector2f newPosition)
{
	PlayerClient::setNewPosition(newPosition);

	m_previousPositions.emplace_back(newPosition, m_moveDirection);

	sf::Packet packetToSend;
	packetToSend << eServerMessageType::ePlayerMoveToPosition << ServerMessagePlayerMove(m_newPosition, m_movementSpeed);
	NetworkHandler::getInstance().sendMessageToServer(packetToSend);
}

GameObjectClient::GameObjectClient(sf::Vector2f startingPosition, float expirationTime, eAnimationName startingAnimationName, eGameObjectType type)
	: m_type(type),
	m_position(startingPosition),
	m_sprite(startingPosition, startingAnimationName),
	m_lifeTimer(expirationTime, true)
{}