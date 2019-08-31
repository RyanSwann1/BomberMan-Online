#include "GameObjects.h"
#include "NetworkHandler.h"
#include "ServerMessageType.h"
#include "ServerMessages.h"
#include "Resources.h"

//Player
PlayerClient::PlayerClient(int tileSize, int ID, sf::Vector2f startingPosition)
	: Player(ID, startingPosition, ePlayerControllerType::eHuman),
	m_shape(sf::Vector2f(static_cast<float>(tileSize), static_cast<float>(tileSize))),
	m_AABB(startingPosition, sf::Vector2f(static_cast<float>(tileSize), static_cast<float>(tileSize)))
{
	m_shape.setPosition(startingPosition);
	m_shape.setFillColor(sf::Color::Red);
}

void PlayerClient::setNewPosition(sf::Vector2f newPosition)
{
	m_newPosition = newPosition;
	m_previousPosition = m_position;
	m_moving = true;

	sf::Packet packetToSend;
	packetToSend << eServerMessageType::ePlayerMoveToPosition << ServerMessagePlayerMove(m_newPosition, m_movementSpeed);
	NetworkHandler::getInstance().sendMessageToServer(packetToSend);
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

//Bomb
Bomb::Bomb(sf::Vector2f startingPosition, float expirationTime)
	: m_position(startingPosition),
	m_sprite(Textures::getInstance().getTileSheet().getTexture(), Textures::getInstance().getTileSheet().getFrameRect(236)),
	m_lifeTimer(expirationTime, true)
{
	m_sprite.setPosition(startingPosition);
}

//Explosion
Explosion::Explosion(sf::Vector2f startingPosition, float expirationTime)
	: m_position(startingPosition),
	m_sprite(Textures::getInstance().getTileSheet().getTexture(), Textures::getInstance().getTileSheet().getFrameRect(272)),
	m_lifeTimer(expirationTime, true)
{
	m_sprite.setPosition(startingPosition);
}

void Explosion::update(float deltaTime)
{
	m_lifeTimer.update(deltaTime);
}

void PlayerClientLocalPlayer::setNewPosition(sf::Vector2f newPosition)
{
	PlayerClient::setNewPosition(newPosition);

	m_previousPositions.push_back(newPosition);
}