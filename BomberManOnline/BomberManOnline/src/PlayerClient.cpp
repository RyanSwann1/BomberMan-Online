#include "PlayerClient.h"
#include "Utilities.h"
#include "ServerMessageType.h"
#include "ServerMessages.h"
#include "NetworkHandler.h"
#include <SFML/Network.hpp>

//Player Client
PlayerClient::PlayerClient(int ID, sf::Vector2f startingPosition, ePlayerType playerType)
	: Player(ID, startingPosition, ePlayerControllerType::eHuman),
	m_playerType(playerType),
	m_sprite(startingPosition, eAnimationName::ePlayerIdleDown),
	m_moveDirection(eDirection::eNone)
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

void PlayerClient::render(sf::RenderWindow& window) const
{
	m_sprite.render(window);
}

void PlayerClient::setNewPosition(sf::Vector2f newPosition, const std::vector<std::vector<eCollidableTile>>& collisionLayer, sf::Vector2i tileSize, 
	std::vector<MovementPoint>& localPlayerPreviousPositions)
{
	if (m_playerType == ePlayerType::eLocal)
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
	else if (m_playerType == ePlayerType::eRemote)
	{
		m_movementFactor = 0.0f;
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