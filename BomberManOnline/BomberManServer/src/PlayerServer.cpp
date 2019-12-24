#include "PlayerServer.h"
#include "Utilities.h"
#include "PathFinding.h"
#include "Server.h"
#include "ServerMessageType.h"
#include "ServerMessages.h"
#include <iostream>
#include <algorithm>
#include <assert.h>

constexpr int PICK_UP_SEARCH_RANGE = 5;
constexpr int MIN_DISTANCE_FROM_ENEMY = 1; // Min distance from enemy to place bomb

//Player Server
PlayerServer::PlayerServer(int ID, sf::Vector2f startingPosition, ePlayerControllerType controllerType)
	: Player(ID, startingPosition, controllerType)
{}

void PlayerServer::setNewPosition(sf::Vector2f newPosition, Server & server)
{
	//Assign new movement direction
	if (newPosition.x > m_position.x)
	{
		m_facingDirection = eDirection::eRight;
	}
	else if (newPosition.x < m_position.x)
	{
		m_facingDirection = eDirection::eLeft;
	}
	else if (newPosition.y < m_position.y)
	{
		m_facingDirection = eDirection::eUp;
	}
	else if (newPosition.y > m_position.y)
	{
		m_facingDirection = eDirection::eDown;
	}

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

	if (!isMoving() && !m_pathToTile.empty())
	{
		setNewPosition(m_pathToTile.back(), m_server);
		m_pathToTile.pop_back();
	}
}

bool PlayerServerAI::placeBomb()
{
	if (Player::placeBomb())
	{
		sf::Packet packetToSend;
		packetToSend << eServerMessageType::ePlaceBomb << m_position.x << m_position.y;
		m_server.broadcastMessage(packetToSend);
		m_server.placeBomb(m_position, m_currentBombExplosionSize);

		return true;
	}

	return false;
}

void PlayerServerAI::handleAIStates(float frameTime)
{
	switch (m_currentState)
	{
	case eAIState::eMakeDecision:
	{
		if (isMoving())
		{
			break;
		}

		//Target Player has been found
		if (m_targetPlayerID != INVALID_PLAYER_ID)
		{
			m_currentState = eAIState::eSetDestinationToTargetPlayer;
		}
		//Search for target Player
		else if (m_behavour == eAIBehaviour::eAggressive && m_targetPlayerID == INVALID_PLAYER_ID)
		{
			for (const auto& targetPlayer : m_server.getPlayers())
			{
				sf::Vector2f targetPlayerPosition = Utilities::getClosestGridPosition(targetPlayer->getPosition(), m_server.getTileSize());
				if (targetPlayer->getID() != m_ID && targetPlayer->getControllerType() == ePlayerControllerType::eHuman &&
					PathFinding::getInstance().isPositionReachable(m_position, targetPlayerPosition, m_server))
				{
					m_targetPlayerID = targetPlayer->getID();
					m_currentState = eAIState::eSetDestinationToTargetPlayer;
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
				m_currentState = eAIState::eSetDestinationAtBox;
			}
			else
			{
#ifdef RENDER_PATHING
				handleRenderPathing();
#endif // RENDER_PATHING
				m_currentState = eAIState::eMovingToPickUp;
			}
		}
	}

	break;
	case eAIState::eMovingToBox:
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
						m_currentState = eAIState::eSetDestinationToTargetPlayer;
						break;
					}
				}
			}
		}

	break;
	case eAIState::eMovingToTargetPlayer:
	{
		if (!isMoving())
		{
			const PlayerServer* targetPlayer = m_server.getPlayer(m_targetPlayerID);
			if (targetPlayer)
			{
				m_currentState = eAIState::eSetDestinationToTargetPlayer;
			}
			else
			{
				m_currentState = eAIState::eMakeDecision;
				m_targetPlayerID = INVALID_PLAYER_ID;
			}
		}
	}

	break;
	case eAIState::eMovingToSafePosition:
		if (!isMoving() && m_pathToTile.empty())
		{
			m_currentState = eAIState::eWait;
		}
		else if (!isMoving() && !m_pathToTile.empty())
		{
			for (sf::Vector2f positionInPath : m_pathToTile)
			{
				if (!PathFinding::getInstance().isPositionInRangeOfAllExplosions(positionInPath, m_server))
				{
					continue;

				}

				m_currentState = eAIState::eSetDestinationAtSafePosition;
				break;
			}
		}

	break;
	case eAIState::eMovingToPickUp:
		if (!isMoving() && m_pathToTile.empty())
		{
			m_currentState = eAIState::eMakeDecision;
		}

	break;
	case eAIState::eSetDestinationAtBox:
	{
		PathFinding::getInstance().getPathToClosestBox(m_position, m_pathToTile, m_server);
		assert(!m_pathToTile.empty());
		if (!m_pathToTile.empty())
		{
#ifdef RENDER_PATHING
			handleRenderPathing();
#endif // RENDER_PATHING
			setNewPosition(m_pathToTile.back(), m_server);
			m_currentState = eAIState::eMovingToBox;
		}
	}

	break;
	case eAIState::eSetDestinationToTargetPlayer:
	{
		if (!isMoving())
		{
			const PlayerServer* targetPlayer = m_server.getPlayer(m_targetPlayerID);
			if (targetPlayer)
			{
				onSetDestinationToTargetPlayer(*targetPlayer);
			}
			else
			{
				m_targetPlayerID = INVALID_PLAYER_ID;
				m_currentState = eAIState::eMakeDecision;
			}
		}
	}

	break;
	case eAIState::eSetDestinationAtSafePosition:
	{
		PathFinding::getInstance().getPathToClosestSafePosition(m_position, m_pathToTile, m_server);
		assert(!m_pathToTile.empty());
		if (!m_pathToTile.empty())
		{
#ifdef RENDER_PATHING
			handleRenderPathing();
#endif // RENDER_PATHING

			setNewPosition(m_pathToTile.back(), m_server);
			m_pathToTile.pop_back();

			m_currentState = eAIState::eMovingToSafePosition;
		}
	}

	break;
	case eAIState::ePlantBomb:
	{
		assert(!isMoving());
		if (!isMoving() && placeBomb())
		{
			m_currentState = eAIState::eSetDestinationAtSafePosition;
		}
	}

	break;
	case eAIState::ePlantAndKickBomb :
	{
		if (!isMoving())
		{
			const PlayerServer* targetPlayer = m_server.getPlayer(m_targetPlayerID);
			assert(targetPlayer);
			if (targetPlayer)
			{
				sf::Vector2f targetPosition = Utilities::getClosestGridPosition(targetPlayer->getPosition(), m_server.getTileSize());
				if (PathFinding::getInstance().getPathToTile(m_position, targetPosition, m_server).size() <= 6 && placeBomb())
				{
					sf::Vector2f kickToPosition = PathFinding::getInstance().getFurthestNonCollidablePosition(m_position, m_facingDirection, m_server);
					m_server.kickBombInDirection(m_position, kickToPosition);
					m_currentState = eAIState::eSetDestinationAtSafePosition;
				}
			}
		}
	}

	break;
	case eAIState::eWait:
	{
		if (PathFinding::getInstance().isPositionInRangeOfAllExplosions(m_position, m_server))
		{
			m_currentState = eAIState::eSetDestinationAtSafePosition;
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

void PlayerServerAI::onSetDestinationToTargetPlayer(const PlayerServer& targetPlayer)
{
	assert(m_currentState == eAIState::eSetDestinationToTargetPlayer);
	sf::Vector2f targetPosition = Utilities::getClosestGridPosition(targetPlayer.getPosition(), m_server.getTileSize());
	if (targetPosition == m_position || Utilities::isPositionAdjacent(m_position, targetPosition, m_server.getTileSize()))
	{
		m_currentState = eAIState::ePlantBomb;
	}
	else
	{
		if (Utilities::getRandomNumber(0, 15) > 12 && Utilities::isTargetInDirectSight(m_position, targetPosition, m_facingDirection) && 
			PathFinding::getInstance().getPathToTile(m_position, targetPosition, m_server).size() <= 6)
		{
			m_currentState = eAIState::ePlantAndKickBomb;
			return;
		}

		//Kick Bomb that is in way of path - Planted by another Player
		std::vector<sf::Vector2f> adjacentPositions;
		adjacentPositions.reserve(4);
		PathFinding::getInstance().getNonCollidableAdjacentPositions(m_position, m_server, adjacentPositions);
		for (sf::Vector2f adjacentPosition : adjacentPositions)
		{
			const BombServer* bomb = m_server.getBomb(adjacentPosition);
			if (bomb && !bomb->isMoving())
			{
				eDirection kickDirection = Utilities::getDirectionToAdjacentFromSourcePosition(m_position, bomb->getPosition());
				sf::Vector2f newBombPosition = PathFinding::getInstance().getFurthestNonCollidablePosition(bomb->getPosition(), kickDirection, m_server);

				if (PathFinding::getInstance().getPathToTile(bomb->getPosition(), newBombPosition, m_server).size() >= bomb->getExplosionSize())
				{
					m_server.kickBombInDirection(bomb->getPosition(), newBombPosition);
					break;
				}
			}		
		}

		PathFinding::getInstance().getPathToTile(m_position, targetPosition, m_pathToTile, m_server);
		assert(!m_pathToTile.empty());
		bool bombFound = false;
		for (sf::Vector2f positionInPath : m_pathToTile)
		{
			const BombServer* bomb = m_server.getBomb(positionInPath);
			if (bomb && bomb->isMoving())
			{
				continue;
			}
			else if (bomb)
			{
				bombFound = true;

				PathFinding::getInstance().getSafePathToTarget(m_position, targetPosition, *bomb, m_server, m_pathToTile);
				if (!m_pathToTile.empty())
				{
#ifdef RENDER_PATHING
					handleRenderPathing();
#endif // RENDER_PATHING
					setNewPosition(m_pathToTile.back(), m_server);
					m_pathToTile.pop_back();
					m_currentState = eAIState::eMovingToTargetPlayer;
				}
				else
				{
					PathFinding::getInstance().getPathToClosestSafePosition(m_position, m_pathToTile, m_server);
					assert(!m_pathToTile.empty());
					if (!m_pathToTile.empty())
					{
#ifdef RENDER_PATHING
						handleRenderPathing();
#endif // RENDER_PATHING
						setNewPosition(m_pathToTile.back(), m_server);
						m_pathToTile.pop_back();
						m_currentState = eAIState::eMovingToSafePosition;
					}
				}

				break;
			}
		}
		if (!bombFound)
		{
#ifdef RENDER_PATHING
			handleRenderPathing();
#endif // RENDER_PATHING
			setNewPosition(m_pathToTile.back(), m_server);
			m_pathToTile.pop_back();
			m_currentState = eAIState::eMovingToTargetPlayer;
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