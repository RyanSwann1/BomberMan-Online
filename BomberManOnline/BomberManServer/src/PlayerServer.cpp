#include "PlayerServer.h"
#include "Utilities.h"
#include "PathFinding.h"
#include "Server.h"
#include "ServerMessageType.h"
#include "ServerMessages.h"
#include <iostream>
#include <assert.h>

constexpr int PICK_UP_SEARCH_RANGE = 5;
constexpr int MIN_DISTANCE_FROM_ENEMY = 1; // Min distance from enemy to place bomb

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
	m_behavour(eAIBehaviour::eAggressive),
	m_currentState(eAIState::eMakeDecision),
	m_pathToTile(),
	m_waitTimer(2.5f),
	m_targetPlayerID(INVALID_PLAYER_ID)
{}

void PlayerServerAI::update(float frameTime)
{
	Player::update(frameTime);
	handleAIStates(frameTime);

	if (!isMoving())
	{
		if(!m_pathToTile.empty())
		{
			setNewPosition(m_pathToTile.back(), m_server);
			m_pathToTile.pop_back();
		}
	}
}

void PlayerServerAI::handleAIStates(float frameTime)
{
	switch (m_currentState)
	{
	case eAIState::eMakeDecision:
	{
		//Target Player has been found
		if (m_targetPlayerID != INVALID_PLAYER_ID)
		{
			const PlayerServer* targetPlayer = m_server.getPlayer(m_targetPlayerID);
			if (targetPlayer)
			{
				sf::Vector2f targetPosition = Utilities::getClosestGridPosition(targetPlayer->getPosition(), m_server.getTileSize());
				PathFinding::getInstance().getPositionClosestToTarget(m_position, targetPosition, m_server, m_pathToTile);
				assert(!m_pathToTile.empty());
				if (!m_pathToTile.empty())
				{
#ifdef RENDER_PATHING
					handleRenderPathing();
#endif // RENDER_PATHING

					setNewPosition(m_pathToTile.back(), m_server);
					m_currentState = eAIState::eMovingToTargetPlayer;
				}
			}
			else
			{
				m_targetPlayerID = INVALID_PLAYER_ID;
				m_currentState = eAIState::eMakeDecision;
			}
		}
		//Search for target Player
		else if (m_behavour == eAIBehaviour::eAggressive && m_targetPlayerID == INVALID_PLAYER_ID)
		{
			for (const auto& targetPlayer : m_server.getPlayers())
			{
				if (targetPlayer->getID() != m_ID && targetPlayer->getControllerType() == ePlayerControllerType::eHuman &&
					PathFinding::getInstance().isPositionReachable(m_position, targetPlayer->getPosition(), m_server))
				{
					m_targetPlayerID = targetPlayer->getID();
					sf::Vector2f targetPosition = Utilities::getClosestGridPosition(targetPlayer->getPosition(), m_server.getTileSize());
					PathFinding::getInstance().getPositionClosestToTarget(m_position, targetPosition, m_server, m_pathToTile);
					assert(!m_pathToTile.empty());
					if (!m_pathToTile.empty())
					{
#ifdef RENDER_PATHING
						handleRenderPathing();
#endif // RENDER_PATHING

						setNewPosition(m_pathToTile.back(), m_server);
						m_currentState = eAIState::eMovingToTargetPlayer;
					}
					break;
				}
			}
		}

		//Target Player not found
		if (m_currentState == eAIState::eMakeDecision)
		{
			PathFinding::getInstance().getPathToClosestPickUp(m_position, m_pathToTile, m_server, PICK_UP_SEARCH_RANGE);
			if (m_pathToTile.empty())
			{
				m_currentState = eAIState::eSetTargetAtBox;
			}
			else
			{
#ifdef RENDER_PATHING
				handleRenderPathing();
#endif // RENDER_PATHING
				m_currentState = eAIState::eMoveToPickUp;
			}
		}
	}

	break;
	case eAIState::eSetTargetAtBox:
	{
		PathFinding::getInstance().getPathToClosestBox(m_position, m_pathToTile, m_server);
		assert(!m_pathToTile.empty());
		if (!m_pathToTile.empty())
		{
#ifdef RENDER_PATHING
			handleRenderPathing();
#endif // RENDER_PATHING
			setNewPosition(m_pathToTile.back(), m_server);
			m_currentState = eAIState::eMoveToBox;
		}
	}

	break;
	case eAIState::eMoveToBox :
		if (!isMoving() && m_pathToTile.empty())
		{
			m_currentState = eAIState::ePlantBomb;
		}
		else if (!isMoving() && !m_pathToTile.empty())
		{
			if (!m_pathToTile.empty() && !Utilities::isPositionNeighbouringBox(m_server.getCollisionLayer(), m_pathToTile.front(),
				m_server.getLevelSize(), m_server.getTileSize()))
			{
				m_currentState = eAIState::eMakeDecision;
			}
			else if (m_behavour == eAIBehaviour::eAggressive && m_targetPlayerID == INVALID_PLAYER_ID)
			{
				for (const auto& player : m_server.getPlayers())
				{
					if (player->getID() != m_ID && player->getControllerType() == ePlayerControllerType::eHuman &&
						PathFinding::getInstance().isPositionReachable(m_position, player->getPosition(), m_server))
					{
						m_targetPlayerID = player->getID();
						m_currentState = eAIState::eSetPositionToTargetPlayer;
						break;
					}
				}
			}
		}

	break;
	case eAIState::eSetPositionToTargetPlayer:
	{
		if (!isMoving())
		{
			const PlayerServer* targetPlayer = m_server.getPlayer(m_targetPlayerID);
			if (targetPlayer)
			{
				sf::Vector2f targetPosition = Utilities::getClosestGridPosition(targetPlayer->getPosition(), m_server.getTileSize());
				PathFinding::getInstance().getPositionClosestToTarget(m_position, targetPosition, m_server, m_pathToTile);
				assert(!m_pathToTile.empty());
				if (!m_pathToTile.empty())
				{
#ifdef RENDER_PATHING
					handleRenderPathing();
#endif // RENDER_PATHING

					setNewPosition(m_pathToTile.back(), m_server);
					m_currentState = eAIState::eMovingToTargetPlayer;
				}
			}
			else
			{
				m_targetPlayerID = INVALID_PLAYER_ID;
				m_currentState = eAIState::eMakeDecision;
			}
		}
	}

	break;
	case eAIState::eSetTargetAtSafePosition:
	{
		PathFinding::getInstance().getPathToClosestSafePosition(m_position, m_pathToTile, m_server);
		if (!m_pathToTile.empty())
		{
#ifdef RENDER_PATHING
			handleRenderPathing();
#endif // RENDER_PATHING

			setNewPosition(m_pathToTile.back(), m_server);
			m_pathToTile.pop_back();

			m_currentState = eAIState::eMoveToSafePosition;
		}
		else
		{
			m_currentState = eAIState::eMakeDecision;
		}
	}

	break;
	case eAIState::eMovingToTargetPlayer :
	{
		if (!isMoving() && m_pathToTile.empty())
		{
			const PlayerServer* targetPlayer = m_server.getPlayer(m_targetPlayerID);
			if (targetPlayer)
			{
				onMovingToTargetPlayerState(*targetPlayer);
			}
			else
			{
				m_currentState = eAIState::eMakeDecision;
				m_targetPlayerID = INVALID_PLAYER_ID;
			}
		}
	}

	break;
	case eAIState::eMoveToSafePosition :
		if (!isMoving() && m_pathToTile.empty())
		{
			m_currentState = eAIState::eWait;
		}

	break;
	case eAIState::eMoveToPickUp :
		if (!isMoving() && m_pathToTile.empty())
		{
			m_currentState = eAIState::eMakeDecision;
		}

	break;
	case eAIState::ePlantBomb:
	{
		if (placeBomb())
		{
			sf::Packet packetToSend;
			packetToSend << eServerMessageType::ePlaceBomb << m_position.x << m_position.y;
			m_server.broadcastMessage(packetToSend);
			m_server.placeBomb(m_position, m_currentBombExplosionSize);

			m_currentState = eAIState::eSetTargetAtSafePosition;
		}
	}

	break;
	case eAIState::eWait:
	{
		if (PathFinding::getInstance().isPositionInRangeOfAllExplosions(m_position, m_server))
		{
			m_currentState = eAIState::eSetTargetAtSafePosition;
			m_waitTimer.setActive(false);
			m_waitTimer.resetElaspedTime();
		}
		else
		{
			m_waitTimer.setActive(true);
			m_waitTimer.update(frameTime);
			if (m_waitTimer.isExpired())
			{
				m_currentState = eAIState::eMakeDecision;
				m_waitTimer.resetElaspedTime();
			}
		}
	}

	break;
	}
}

void PlayerServerAI::onMovingToTargetPlayerState(const PlayerServer& targetPlayer)
{
	sf::Vector2i tileSize = m_server.getTileSize();
	sf::Vector2f targetPosition = Utilities::getClosestGridPosition(targetPlayer.getPosition(), tileSize);

	if (Utilities::isPositionAdjacent(m_position, targetPosition, tileSize))
	{
		m_currentState = eAIState::ePlantBomb;
	}
	else
	{
		bool bombFound = false;
		for (sf::Vector2f position : PathFinding::getInstance().getPathToTile(m_position, targetPosition, m_server))
		{
			const BombServer* bomb = m_server.getBomb(position);
			if (bomb && PathFinding::getInstance().isPositionInRangeOfExplosion(m_position, *bomb, m_server))
			{
				bombFound = true;
				sf::Vector2i tileSize = m_server.getTileSize();

				PathFinding::getInstance().getSafePositionClosestToTarget(m_position, targetPosition, m_server, m_pathToTile);
				if (!m_pathToTile.empty())
				{
#ifdef RENDER_PATHING
					handleRenderPathing();
#endif // RENDER_PATHING
					std::cout << "Redirect path to safe path\n";
					setNewPosition(m_pathToTile.back(), m_server);
					m_pathToTile.pop_back();
					m_currentState = eAIState::eSetPositionToTargetPlayer;

					break;
				}
				else
				{
					PathFinding::getInstance().getPathToClosestSafePosition(m_position, *bomb, m_pathToTile, m_server);
					assert(!m_pathToTile.empty());
					if (!m_pathToTile.empty())
					{
#ifdef RENDER_PATHING
						handleRenderPathing();
#endif // RENDER_PATHING
						std::cout << "Move to safety\n";
						setNewPosition(m_pathToTile.back(), m_server);
						m_pathToTile.pop_back();
						m_currentState = eAIState::eMoveToSafePosition;

						break;
					}
				}
			}
		}

		if (!bombFound)
		{
			m_currentState = eAIState::eSetPositionToTargetPlayer;
		}
	}
}

#ifdef RENDER_PATHING
void PlayerServerAI::handleRenderPathing()
{
	sf::Packet packetToSend;
	packetToSend << eServerMessageType::ePathToRender << m_pathToTile << m_ID;
	m_server.broadcastMessage(packetToSend);
}
#endif // RENDER_PATHING

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