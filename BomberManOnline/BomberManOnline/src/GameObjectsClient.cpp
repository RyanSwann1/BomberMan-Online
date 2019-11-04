#include "GameObjectsClient.h"
#include "NetworkHandler.h"
#include "ServerMessageType.h"
#include "ServerMessages.h"
#include "Resources.h"
#include "Utilities.h"
#include "Level.h"

//Player Client
PlayerClient::PlayerClient(int ID, sf::Vector2f startingPosition)
	: Player(ID, startingPosition, ePlayerControllerType::eHuman),
	m_sprite(startingPosition, eAnimationName::ePlayerIdleDown)
{}

void PlayerClient::update(float deltaTime)
{
	Player::update(deltaTime);

	if (m_moving)
	{
		m_sprite.setPosition(m_position);
		m_sprite.update(deltaTime);
	}
}

void PlayerClient::render(sf::RenderWindow & window) const
{
	m_sprite.render(window);
}

void PlayerClient::setRemotePlayerPosition(sf::Vector2f newPosition)
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

void PlayerClient::setLocalPlayerPosition(sf::Vector2f newPosition, const std::vector<std::vector<eCollidableTile>>& collisionLayer, sf::Vector2i tileSize,
	std::vector<MovementPoint>& localPlayerPreviousPositions)
{
	if (!m_moving && !Utilities::isPositionCollidable(collisionLayer, newPosition, tileSize))
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

		sf::Packet packetToSend;
		packetToSend << eServerMessageType::ePlayerMoveToPosition << ServerMessagePlayerMove(m_newPosition, m_movementSpeed);
		NetworkHandler::getInstance().sendMessageToServer(packetToSend);

		localPlayerPreviousPositions.emplace_back(m_newPosition, m_moveDirection);
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

void PlayerClient::stopAtPosition(sf::Vector2f position)
{
	m_position = position;
	m_previousPosition = position;
	m_moving = false;
	m_movementFactor = 0.0f;
}

GameObjectClient::GameObjectClient(sf::Vector2f startingPosition, float expirationTime, eAnimationName startingAnimationName, eGameObjectType type, 
	eTimerActive timerActive)
	: GameObject(startingPosition, expirationTime, type),
	m_sprite(startingPosition, startingAnimationName)
{}

void GameObjectClient::render(sf::RenderWindow & window) const
{
	m_sprite.render(window);
}

void GameObjectClient::update(float deltaTime)
{
	m_sprite.update(deltaTime);
}