#include "GameObjects.h"
#include "NetworkHandler.h"
#include "ServerMessageType.h"
#include "ServerMessages.h"
#include "Resources.h"

//Player
PlayerClient::PlayerClient(int tileSize, int ID, sf::Vector2f startingPosition)
	: Player(ID, startingPosition, ePlayerControllerType::eHuman),
	m_sprite(startingPosition, eAnimationName::ePlayerIdleDown, 0.5f),
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

//Bomb
BombClient::BombClient(sf::Vector2f startingPosition, float expirationTime)
	: m_position(startingPosition),
	m_sprite(Textures::getInstance().getTileSheet().getTexture(), Textures::getInstance().getTileSheet().getFrameRect(236)),
	m_lifeTimer(expirationTime, true)
{
	m_sprite.setPosition(startingPosition);
}

//Explosion
Explosion::Explosion(sf::Vector2f startingPosition, float expirationTime)
	: m_position(startingPosition),
	m_sprite(startingPosition, eAnimationName::eExplosion, 0.5f),
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

	m_previousPositions.emplace_back(newPosition, m_moveDirection);

	sf::Packet packetToSend;
	packetToSend << eServerMessageType::ePlayerMoveToPosition << ServerMessagePlayerMove(m_newPosition, m_movementSpeed);
	NetworkHandler::getInstance().sendMessageToServer(packetToSend);
}