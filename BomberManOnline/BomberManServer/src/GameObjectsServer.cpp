#include "GameObjectsServer.h"
#include "Utilities.h"
#include "PathFinding.h"
#include "Server.h"
#include "ServerMessageType.h"
#include "ServerMessages.h"
#include <iostream>
#include <assert.h>

PlayerServerAI::PlayerServerAI(int ID, sf::Vector2f startingPosition, ePlayerControllerType controllerType, Server & server)
	: Player(ID, startingPosition, controllerType),
	m_server(server),
	m_behavour(eAIBehaviour::eAggressive),
	m_currentState(eAIState::eMakeDecision),
	m_pathToTile(),
	m_waitTimer(2.5f)
{}

void PlayerServerAI::update(float frameTime)
{
	Player::update(frameTime);

	const auto& collisionLayer = m_server.getCollisionLayer();
	const auto& players = m_server.getPlayers();
	sf::Vector2i tileSize = m_server.getTileSize();
	sf::Vector2i levelSize = m_server.getLevelSize();

	switch (m_currentState)
	{
	case eAIState::eMakeDecision:
	{
		bool targetFound = false;
		if (m_behavour == eAIBehaviour::eAggressive)
		{
			for (const auto& targetPlayer : m_server.getPlayers())
			{
				if (targetPlayer->getID() == m_ID)
				{
					continue;
				}

				if (PathFinding::getInstance().isPositionReachable(m_position, targetPlayer->getPosition(), m_server))
				{
					targetFound = true;
					m_currentState = eAIState::eMoveToNearestPlayer;
					break;
				}
			}
		}
		if (!targetFound || m_behavour == eAIBehaviour::ePassive)
		{
			PathFinding::getInstance().pathToClosestBox(m_position, m_pathToTile, m_server);
			if (!m_pathToTile.empty())
			{
				std::cout << "Move To Box\n";
				m_currentState = eAIState::eMoveToBox;
				m_moving = true;

				m_newPosition = m_pathToTile.back();
				m_pathToTile.pop_back();
				m_previousPosition = m_position;

				sf::Packet globalPacket;
				globalPacket << static_cast<int>(eServerMessageType::eNewPlayerPosition) << m_newPosition.x << m_newPosition.y << m_ID;
				m_server.broadcastMessage(globalPacket);
			}
		}
	}

	break;
	case eAIState::eMoveToBox:
	{
		m_movementFactor += frameTime * m_movementSpeed;
		m_position = Utilities::Interpolate(m_previousPosition, m_newPosition, m_movementFactor);

		if (m_position == m_newPosition)
		{
			std::cout << "Reached new position\n";
			m_movementFactor = 0;
			m_previousPosition = m_position;

			if (m_pathToTile.empty())
			{
				m_moving = false;
				m_currentState = eAIState::ePlantBomb;
			}
			else
			{
				m_moving = true;
				m_newPosition = m_pathToTile.back();
				if (!Utilities::isPositionNeighbouringBox(collisionLayer, m_pathToTile.front(), tileSize, levelSize))
				{
					m_moving = false;
					m_currentState = eAIState::eMakeDecision;
					std::cout << "Box Not Found\n";
				}

				m_pathToTile.pop_back();
			}

			sf::Packet globalPacket;
			globalPacket << static_cast<int>(eServerMessageType::eNewPlayerPosition) << m_newPosition.x << m_newPosition.y << m_ID;
			m_server.broadcastMessage(globalPacket);
		}
	}

	break;
	case eAIState::eMoveToNearestPlayer:
	{
		if (m_position == m_newPosition)
		{
			float distance = levelSize.x * levelSize.y;
			int closestTargetPlayerID = 0;

			for (const auto& target : players)
			{
				//Don't target self
				if (target->getID() == m_ID)
				{
					continue;
				}

				if (Utilities::distance(m_position, target->getPosition(), tileSize) < distance)
				{
					closestTargetPlayerID = target->getID();
					distance = Utilities::distance(m_position, target->getPosition(), tileSize);
				}
			}

			auto cIter = std::find_if(players.cbegin(), players.cend(), [closestTargetPlayerID](const auto& target) { return target->m_ID == closestTargetPlayerID; });
			assert(cIter != players.cend());
			sf::Vector2f positionToMoveTo = PathFinding::getInstance().getPositionClosestToTarget(m_position, (*cIter)->getPosition(), m_server);
		}


		m_movementFactor += frameTime * m_movementSpeed;
		m_position = Utilities::Interpolate(m_previousPosition, m_newPosition, m_movementFactor);

		if (m_position == m_newPosition)
		{
			m_movementFactor = 0;

			if (m_pathToTile.empty())
			{
				m_moving = false;
				m_currentState = eAIState::ePlantBomb;
			}
			else
			{
				m_moving = true;
				m_newPosition = m_pathToTile.back();
				m_pathToTile.pop_back();
			}

			sf::Packet globalPacket;
			globalPacket << static_cast<int>(eServerMessageType::eNewPlayerPosition) << m_position.x << m_position.y << m_ID;
			m_server.broadcastMessage(globalPacket);
		}
	}

	break;
	case eAIState::eSetPositionAtSafeArea:
	{
		PathFinding::getInstance().pathToClosestSafePosition(m_position, m_pathToTile, m_server);
		m_currentState = eAIState::eMoveToSafePosition;
		m_moving = true;

		m_newPosition = m_pathToTile.back();
		m_pathToTile.pop_back();
		m_previousPosition = m_position;
	}

	break;
	case eAIState::eMoveToSafePosition:
	{
		m_movementFactor += frameTime * m_movementSpeed;
		m_position = Utilities::Interpolate(m_previousPosition, m_newPosition, m_movementFactor);

		if (m_position == m_newPosition)
		{
			m_movementFactor = 0;

			if (m_pathToTile.empty())
			{
				m_moving = false;
				m_currentState = eAIState::eWait;
			}
			else
			{
				m_moving = true;
				m_newPosition = m_pathToTile.back();
				m_pathToTile.pop_back();
			}

			sf::Packet globalPacket;
			globalPacket << static_cast<int>(eServerMessageType::eNewPlayerPosition) << m_position.x << m_position.y << m_ID;
			m_server.broadcastMessage(globalPacket);
		}
	}

	break;
	case eAIState::ePlantBomb:
	{
		if (m_bombPlacementTimer.isExpired())
		{
			ServerMessageBombPlacement bombPlacementMessage;
			bombPlacementMessage.position = m_position;
			bombPlacementMessage.lifeTimeDuration = m_bombPlacementTimer.getExpirationTime();

			sf::Packet packetToSend;
			packetToSend << eServerMessageType::ePlaceBomb << bombPlacementMessage;
			m_server.broadcastMessage(packetToSend);
			m_server.placeBomb(m_position, m_bombPlacementTimer.getExpirationTime());

			m_currentState = eAIState::eSetPositionAtSafeArea;
		}
	}

	break;
	case eAIState::eWait:
	{
		m_waitTimer.setActive(true);
		m_waitTimer.update(frameTime);
		if (m_waitTimer.isExpired())
		{
			m_currentState = eAIState::eMakeDecision;
			m_waitTimer.resetElaspedTime();
		}
	}

	break;
	}
}

PlayerServerHuman::~PlayerServerHuman()
{
	std::cout << "Destroyed Human Player\n";
	m_tcpSocket->disconnect();
}