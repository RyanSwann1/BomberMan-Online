#include "PlayerServer.h"
#include "Utilities.h"
#include "PathFinding.h"
#include "Server.h"
#include "ServerMessageType.h"
#include "ServerMessages.h"
#include <iostream>
#include <assert.h>

constexpr int PICK_UP_SEARCH_RANGE = 5;

//Player Server
PlayerServer::PlayerServer(int ID, sf::Vector2f startingPosition, ePlayerControllerType controllerType)
	: Player(ID, startingPosition, controllerType)
{}

void PlayerServer::setNewPosition(sf::Vector2f newPosition, Server & server)
{
	m_newPosition = newPosition;
	m_previousPosition = m_position;

	sf::Packet globalPacket;
	globalPacket << static_cast<int>(eServerMessageType::eNewPlayerPosition) << m_newPosition.x << m_newPosition.y << m_ID;
	server.broadcastMessage(globalPacket);
}

//Player AI
PlayerServerAI::PlayerServerAI(int ID, sf::Vector2f startingPosition, Server& server)
	: PlayerServer(ID, startingPosition, ePlayerControllerType::eAI),
	m_server(server),
	m_behavour(eAIBehaviour::ePassive),
	m_currentState(eAIState::eMakeDecision),
	m_pathToTile(),
	m_waitTimer(2.5f)
{}

void PlayerServerAI::update(float frameTime)
{
	handleAIStates(frameTime);
	m_bombPlacementTimer.update(frameTime);

	if (isMoving())
	{
		m_movementFactor += frameTime * m_movementSpeed;
		m_position = Utilities::Interpolate(m_previousPosition, m_newPosition, m_movementFactor);
	}

	if (m_position == m_newPosition)
	{
		m_movementFactor = 0.0f;

		if (m_pathToTile.empty())
		{
			if (m_currentState == eAIState::eMoveToBox)
			{
				m_currentState = eAIState::ePlantBomb;
			}
			else if (m_currentState == eAIState::eMoveToSafePosition)
			{
				m_currentState = eAIState::eWait;
			}
			else if (m_currentState == eAIState::eMoveToPickUp)
			{
				m_currentState = eAIState::eMakeDecision;
			}
		}
		else
		{
			if (m_currentState == eAIState::eMoveToBox)
			{
				if (!Utilities::isPositionNeighbouringBox(m_server.getCollisionLayer(), m_pathToTile.front(), 
					m_server.getLevelSize(), m_server.getTileSize()))
				{
					m_currentState = eAIState::eMakeDecision;
				}
			}

			setNewPosition(m_pathToTile.back(), m_server);
			m_pathToTile.pop_back();
		}
	}
}

void PlayerServerAI::handleAIStates(float frameTime)
{
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
				if (targetPlayer->getID() != m_ID && PathFinding::getInstance().isPositionReachable(m_position, targetPlayer->getPosition(), m_server))
				{
					targetFound = true;
					m_currentState = eAIState::eSetTargetAtNearestPlayer;
					break;
				}
			}
		}
		if (!targetFound || m_behavour == eAIBehaviour::ePassive)
		{
			//PathFinding::getInstance().getPathToClosestPickUp(m_position, m_pathToTile, m_server, PICK_UP_SEARCH_RANGE);
			//(m_pathToTile.empty() ? m_currentState = eAIState::eSetTargetAtBox : m_currentState = eAIState::eMoveToPickUp);
		
			m_currentState = eAIState::eSetTargetAtBox;
		}
	}

	break;
	case eAIState::eSetTargetAtBox:
	{
		PathFinding::getInstance().getPathToClosestBox(m_position, m_pathToTile, m_server);
		assert(!m_pathToTile.empty());
		setNewPosition(m_pathToTile.back(), m_server);
		m_currentState = eAIState::eMoveToBox;
	}

	break;
	case eAIState::eSetTargetAtNearestPlayer:
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

			auto cIter = std::find_if(players.cbegin(), players.cend(), [closestTargetPlayerID](const auto& target) { return target->getID() == closestTargetPlayerID; });
			assert(cIter != players.cend());
			sf::Vector2f positionToMoveTo = PathFinding::getInstance().getPositionClosestToTarget(m_position, (*cIter)->getPosition(), m_server);
		}

		m_movementFactor += frameTime * m_movementSpeed;
		m_position = Utilities::Interpolate(m_previousPosition, m_newPosition, m_movementFactor);

		if (m_position == m_newPosition)
		{
			m_movementFactor = 0.0f;

			if (m_pathToTile.empty())
			{
				m_currentState = eAIState::ePlantBomb;
			}
			else
			{
				m_newPosition = m_pathToTile.back();
				m_pathToTile.pop_back();
			}

			sf::Packet globalPacket;
			globalPacket << static_cast<int>(eServerMessageType::eNewPlayerPosition) << m_position.x << m_position.y << m_ID;
			m_server.broadcastMessage(globalPacket);
		}
	}

	break;
	case eAIState::eSetTargetAtSafePosition:
	{
		PathFinding::getInstance().getPathToClosestSafePosition(m_position, m_pathToTile, m_server);
		assert(!m_pathToTile.empty());
		m_currentState = eAIState::eMoveToSafePosition;
		setNewPosition(m_pathToTile.back(), m_server);
		m_pathToTile.pop_back();
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
			m_server.placeBomb(m_position);

			m_currentState = eAIState::eSetTargetAtSafePosition;
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

//Player Human
PlayerServerHuman::PlayerServerHuman(std::unique_ptr<sf::TcpSocket> tcpSocket, int ID, sf::Vector2f startingPosition, sf::SocketSelector& socketSelector)
	: PlayerServer(ID, startingPosition, ePlayerControllerType::eHuman),
	m_tcpSocket(std::move(tcpSocket))
{ 
	socketSelector.add(*m_tcpSocket.get());
}

PlayerServerHuman::~PlayerServerHuman()
{
	std::cout << "Destroyed Human Player\n";
	m_tcpSocket->disconnect();
}

std::unique_ptr<sf::TcpSocket>& PlayerServerHuman::getTCPSocket()
{
	return m_tcpSocket;
}