#include "PlayerServerAI.h"
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

constexpr int MAX_SAFE_POSITIONS = 5;
constexpr int MAX_BOX_OPTIONS = 5;

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
			if (!m_pathToTile.empty() && 
				!m_server.getTileManager().isPositionAdjacentToBox(Utilities::convertToGridPosition(m_pathToTile.front(), m_server.getTileSize())))
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
	case eAIState::eMovingToPositionToPlantBomb :
		if (!isMoving() && m_pathToTile.empty())
		{
			m_currentState = eAIState::ePlantBomb;
		}
		
	break;
	case eAIState::eSetDestinationAtBox:
	{
		if (!isMoving())
		{
			PathFinding::getInstance().getPathToClosestBox(m_position, m_pathToTile, m_server, MAX_BOX_OPTIONS);
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
		if (!isMoving())
		{
			PathFinding::getInstance().getPathToRandomLocalSafePosition(m_position, m_pathToTile, m_server, MAX_SAFE_POSITIONS);
			assert(!m_pathToTile.empty());

			for (const auto& bomb : m_server.getBombs())
			{
				if (Utilities::isPositionAdjacent(bomb.getPosition(), m_pathToTile.front(), m_server.getTileSize()))
				{
					int i = 0;
				}
			}

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
	}

	break;
	case eAIState::eSetDestinationToPlantBomb :
	{
		if (!isMoving())
		{
			PathFinding::getInstance().getPathToRandomLocalSafePosition(m_position, m_pathToTile, m_server, MAX_SAFE_POSITIONS);
			assert(!m_pathToTile.empty());
			if (!m_pathToTile.empty())
			{
				sf::Vector2f positionToMoveTo = m_pathToTile[Utilities::getRandomNumber(0, m_pathToTile.size() - 1)];
				m_pathToTile.clear();
				PathFinding::getInstance().getPathToTile(m_position, positionToMoveTo, m_pathToTile, m_server);
				assert(!m_pathToTile.empty());
				if (!m_pathToTile.empty())
				{
#ifdef RENDER_PATHING
					handleRenderPathing();
#endif // RENDER_PATHING

					setNewPosition(m_pathToTile.back(), m_server);
					m_pathToTile.pop_back();

					m_currentState = eAIState::eMovingToPositionToPlantBomb;
				}
			}
		}
	}

	break;
	case eAIState::ePlantBomb:
	{
		if (!isMoving() && placeBomb())
		{
			m_currentState = eAIState::eSetDestinationAtSafePosition;
		}
	}

	break;
	case eAIState::ePlantAndKickBomb:
	{
		if (!isMoving())
		{
			const PlayerServer* targetPlayer = m_server.getPlayer(m_targetPlayerID);
			assert(targetPlayer);
			if (targetPlayer)
			{
				sf::Vector2f targetPosition = Utilities::getClosestGridPosition(targetPlayer->getPosition(), m_server.getTileSize());

				if (PathFinding::getInstance().getPathToTile(m_position, targetPosition, m_server).size() <= MAX_KICK_RANGE && placeBomb())
				{
					sf::Vector2f kickToPosition = PathFinding::getInstance().getFurthestNonCollidablePosition(m_position, m_facingDirection, m_server, MAX_KICK_RANGE);
					m_server.kickBombInDirection(m_position, kickToPosition);
					m_currentState = eAIState::eSetDestinationAtSafePosition;
				}
			}
		}
	}

	break;
	case eAIState::eWait:
	{
		if (!isMoving())
		{
			if (PathFinding::getInstance().isPositionInRangeOfAllExplosions(m_position, m_server))
			{
				m_waitTimer.resetElaspedTime();
				m_waitTimer.setActive(false);
			
				if (m_targetPlayerID != INVALID_PLAYER_ID && Utilities::getRandomNumber(0, 10) >= 4)
				{
					m_currentState = eAIState::eSetDestinationToPlantBomb;
				}
				else
				{
					m_currentState = eAIState::eSetDestinationAtSafePosition;
				}
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
		if (Utilities::getRandomNumber(0, 15) > 13 && Utilities::isTargetInDirectSight(m_position, targetPosition, m_facingDirection) &&
			PathFinding::getInstance().getPathToTile(m_position, targetPosition, m_server).size() <= MAX_KICK_RANGE)
		{
			m_currentState = eAIState::ePlantAndKickBomb;
			return;
		}

		if (Utilities::getRandomNumber(0, 10) > 8)
		{
			//Kick Bomb that is in way of path - Planted by another Player
			std::vector<sf::Vector2f> adjacentPositions;
			adjacentPositions.reserve(4);
			PathFinding::getInstance().getNonCollidableAdjacentPositions(m_position, m_server, adjacentPositions);
			for (sf::Vector2f adjacentPosition : adjacentPositions)
			{
				const BombServer* bomb = m_server.getBomb(adjacentPosition);
				if (bomb && !bomb->isMoving())
				{
					eDirection kickDirection = Utilities::getDirectionToAdjacentFromPosition(m_position, bomb->getPosition());
					sf::Vector2f newBombPosition = PathFinding::getInstance().getFurthestNonCollidablePosition(bomb->getPosition(), kickDirection, m_server, MAX_KICK_RANGE);

					if (PathFinding::getInstance().getPathToTile(bomb->getPosition(), newBombPosition, m_server).size() >= bomb->getExplosionSize())
					{
						m_server.kickBombInDirection(bomb->getPosition(), newBombPosition);
						break;
					}
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

				PathFinding::getInstance().getSafePathToTile(m_position, targetPosition, *bomb, m_pathToTile, m_server);
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
#ifdef RENDER_PATHING
					handleRenderPathing();
#endif // RENDER_PATHING
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