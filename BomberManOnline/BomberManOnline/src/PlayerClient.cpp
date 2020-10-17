#include "PlayerClient.h"
#include "Utilities.h"
#include "ServerMessageType.h"
#include "ServerMessages.h"
#include "NetworkHandler.h"
#include "TileManager.h"
#include <SFML/Network.hpp>
#include <assert.h>

//Movemnet Point
MovementPoint::MovementPoint(sf::Vector2f position, eDirection moveDirection)
	: position(position),
	moveDirection(moveDirection)
{}

//Player Client
PlayerClient::PlayerClient(int ID, sf::Vector2f startingPosition, ePlayerType playerType)
	: Player(ID, startingPosition, ePlayerControllerType::eHuman),
	m_playerType(playerType),
	m_sprite(startingPosition, eAnimationName::ePlayerIdleDown)
{}

void PlayerClient::update(float deltaTime)
{
	Player::update(deltaTime);

	if (isMoving())
	{
		m_sprite.setPosition(m_position);
		m_sprite.update(deltaTime);
	}
}

void PlayerClient::render(sf::RenderWindow& window) const
{
	m_sprite.render(window);
}

void PlayerClient::setNewPosition(sf::Vector2f newPosition, const TileManager& tileManager, sf::Vector2i tileSize, 
	std::vector<MovementPoint>& localPlayerPreviousPositions)
{
	if (m_playerType == ePlayerType::eLocal)
	{
		if (!isMoving() && !tileManager.isPositionCollidable(Utilities::convertToGridPosition(newPosition, tileSize)))
		{
			m_newPosition = newPosition;
			m_previousPosition = m_position;

			//Assign new movement direction
			if (newPosition.x > m_position.x)
			{
				m_facingDirection = eDirection::eRight;
				m_sprite.setNewAnimation(eAnimationName::ePlayerMoveRight);
			}
			else if (newPosition.x < m_position.x)
			{
				m_facingDirection = eDirection::eLeft;
				m_sprite.setNewAnimation(eAnimationName::ePlayerMoveLeft);
			}
			else if (newPosition.y < m_position.y)
			{
				m_facingDirection = eDirection::eUp;
				m_sprite.setNewAnimation(eAnimationName::ePlayerMoveUp);
			}
			else if (newPosition.y > m_position.y)
			{
				m_facingDirection = eDirection::eDown;
				m_sprite.setNewAnimation(eAnimationName::ePlayerMoveDown);
			}

			sf::Packet packetToSend;
			packetToSend << eServerMessageType::ePlayerMoveToPosition << ServerMessagePlayerMove(m_newPosition, m_movementSpeed);
			NetworkHandler::getInstance().sendMessageToServer(packetToSend);

			localPlayerPreviousPositions.emplace_back(m_newPosition, m_facingDirection);
		}
	}
	else if (m_playerType == ePlayerType::eRemote)
	{
		m_movementFactor = 0.0f;
		m_newPosition = newPosition;
		m_previousPosition = m_position;

		//Assign new movement direction
		if (newPosition.x > m_position.x)
		{
			m_facingDirection = eDirection::eRight;
			m_sprite.setNewAnimation(eAnimationName::ePlayerMoveRight);
		}
		else if (newPosition.x < m_position.x)
		{
			m_facingDirection = eDirection::eLeft;
			m_sprite.setNewAnimation(eAnimationName::ePlayerMoveLeft);
		}
		else if (newPosition.y < m_position.y)
		{
			m_facingDirection = eDirection::eUp;
			m_sprite.setNewAnimation(eAnimationName::ePlayerMoveUp);
		}
		else if (newPosition.y > m_position.y)
		{
			m_facingDirection = eDirection::eDown;
			m_sprite.setNewAnimation(eAnimationName::ePlayerMoveDown);
		}
	}
}

void PlayerClient::stopAtPosition(sf::Vector2f position)
{
	m_position = position;
	m_previousPosition = position;
	m_movementFactor = 0.0f;
}

#ifdef RENDER_PATHING
void PlayerClient::renderPath(sf::RenderWindow& window) const
{
	for (const auto& node : m_path)
	{
		window.draw(node);
	}
}

void PlayerClient::setPathToRender(const std::vector<sf::Vector2f>& path)
{
	m_path.clear();

	if (!path.empty())
	{
		for (sf::Vector2f i : path)
		{
			sf::RectangleShape shape;
			shape.setPosition(i);
			shape.setFillColor(sf::Color::Red);
			sf::Vector2i tileSize = Textures::getInstance().getTileSheet().getTileSize();
			shape.setSize(sf::Vector2f(tileSize.x, tileSize.y));

			m_path.push_back(shape);
		}

		m_path.front().setFillColor(sf::Color::Green);
	}
}
#endif // DEBUG