#include "GameObjects.h"
#include "NetworkHandler.h"
#include "ServerMessageType.h"
#include "ServerMessages.h"
#include "Resources.h"

//Player
Player::Player(int tileSize, int ID)
	: m_ID(ID),
	m_position(),
	m_newPosition(),
	m_movementSpeed(2.5f),
	m_movementFactor(0.0f),
	m_shape(sf::Vector2f(tileSize, tileSize)),
	m_AABB(m_position, sf::Vector2f(tileSize, tileSize)),
	m_moving(false),
	m_bombPlacementTimer(2.0f, true)
{
	m_shape.setPosition(m_position);
	m_shape.setFillColor(sf::Color::Red);
}

Player::Player(int tileSize, int ID, sf::Vector2f startingPosition)
	: m_ID(ID),
	m_position(startingPosition),
	m_newPosition(),
	m_movementSpeed(2.5f),
	m_movementFactor(0.0f),
	m_shape(sf::Vector2f(tileSize, tileSize)),
	m_AABB(m_position, sf::Vector2f(tileSize, tileSize)),
	m_moving(false),
	m_bombPlacementTimer(2.0f, true)
{
	m_shape.setPosition(m_position);
	m_shape.setFillColor(sf::Color::Red);
}

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

//Bomb
Bomb::Bomb(sf::Vector2f startingPosition, float expirationTime)
	: m_position(startingPosition),
	m_sprite(Textures::, texture.getFrameRect(236)),
	m_lifeTimer(expirationTime, true)
{
	m_sprite.setPosition(startingPosition);
}