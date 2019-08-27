#include "Player.h"
#include "NetworkHandler.h"
#include "ServerMessageType.h"
#include "ServerMessages.h"

void Player::setNewPosition(sf::Vector2f newPosition)
{
	m_newPosition = newPosition;
	m_previousPosition = m_position;
	m_moving = true;

	sf::Packet packetToSend;
	packetToSend << eServerMessageType::ePlayerMoveToPosition << ServerMessagePlayerMove(m_newPosition, m_movementSpeed);
	NetworkHandler::getInstance().sendMessageToServer(packetToSend);
}

void Player::plantBomb()
{
	if (!m_moving && m_bombPlacementTimer.isExpired())
	{
		m_bombPlacementTimer.resetElaspedTime();

		sf::Packet packetToSend;
		packetToSend << eServerMessageType::ePlayerBombPlacementRequest << m_position.x << m_position.y;
		NetworkHandler::getInstance().sendMessageToServer(packetToSend);
	}
}