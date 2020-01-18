#include "LevelCollapser.h"
#include "Server.h"
#include "TileID.h"
#include "Utilities.h"
#include "ServerMessageType.h"
#include "Direction.h"

LevelCollapser::LevelCollapser()
	: m_elaspedTimeUntilStart(1.0f),
	m_startingPosition(),
	m_incrementAmount(14),
	m_currentAmount(0),
	m_currentPlacementPosition(),
	m_placementDirection(eDirection::eRight),
	m_firstPass(true),
	m_disabled(true),
	m_placementTimer(0.1f)
{}

void LevelCollapser::activate(sf::Vector2f startingPosition)
{
	m_startingPosition = startingPosition;
	m_currentPlacementPosition = m_startingPosition;
	m_disabled = false;
}

void LevelCollapser::update(Server& server, TileManager& tileManager, float frameTime)
{
	if (!m_disabled && server.getElaspedTime() >= m_elaspedTimeUntilStart)
	{
		m_placementTimer.setActive(true);
		m_disabled = false;
	}
	else if(m_disabled)
	{
		return;
	}
	
	m_placementTimer.update(frameTime);
	if (m_placementTimer.isExpired())
	{
		m_placementTimer.resetElaspedTime();
		placeNextCollidableTile(server, tileManager);
	}

	//Destroy Player, pick up or Bomb that is now occupying the same position 
	//as newly spawned collidable tile
}

void LevelCollapser::placeNextCollidableTile(Server& server, TileManager& tileManager)
{
	sf::Vector2i tileSize = server.getTileSize();
	if (tileManager.isTileOnPosition(eTileID::eBlank, Utilities::convertToGridPosition(m_startingPosition, tileSize)))
	{
		tileManager.changeTile(eTileID::eWall, Utilities::convertToGridPosition(m_startingPosition, tileSize));

		sf::Packet packetToSend;
		packetToSend << eServerMessageType::eNewCollidableTile << m_currentPlacementPosition;
		server.broadcastMessage(packetToSend);
	}
	else
	{
		if (m_firstPass)
		{
			switch (m_placementDirection)
			{
			case eDirection::eRight:
			{
				if (m_currentAmount < m_incrementAmount)
				{
					++m_currentAmount;
					m_currentPlacementPosition.x += tileSize.x;
				}
				else
				{
					m_currentAmount = 0;
					m_placementDirection = eDirection::eDown;
				}
			}
			break;
			case eDirection::eDown:
			{
				if (m_currentAmount < m_incrementAmount)
				{
					++m_currentAmount;
					m_currentPlacementPosition.y += tileSize.y;
				}
				else
				{
					m_currentAmount = 0;
					m_placementDirection = eDirection::eLeft;
				}
			}
			break;
			case eDirection::eLeft:
			{
				if (m_currentAmount < m_incrementAmount)
				{
					++m_currentAmount;
					m_currentPlacementPosition.x -= tileSize.x;
				}
				else
				{
					m_currentAmount = 0;
					m_placementDirection = eDirection::eUp;
				}
			}
			break;
			case eDirection::eUp:
			{
				if (m_currentAmount < m_incrementAmount - 1)
				{
					++m_currentAmount;
					m_currentPlacementPosition.y -= tileSize.y;
				}
				else
				{
					m_currentAmount = 0;
					m_firstPass = false;
					--m_incrementAmount;
					m_placementDirection = eDirection::eRight;

				}
			}
			break;
			}
		}
		else
		{
			switch (m_placementDirection)
			{
			case eDirection::eRight:
			{
				if (m_currentAmount < m_incrementAmount)
				{
					++m_currentAmount;
					m_currentPlacementPosition.x += tileSize.x;
				}
				else
				{
					m_currentAmount = 0;
					--m_incrementAmount;
					m_placementDirection = eDirection::eDown;
				}
			}
			break;
			case eDirection::eDown:
			{
				if (m_currentAmount < m_incrementAmount)
				{
					++m_currentAmount;
					m_currentPlacementPosition.y += tileSize.y;
				}
				else
				{
					m_currentAmount = 0;
					m_placementDirection = eDirection::eLeft;
				}
			}
			break;
			case eDirection::eLeft:
			{
				if (m_currentAmount < m_incrementAmount)
				{
					++m_currentAmount;
					m_currentPlacementPosition.x -= tileSize.x;
				}
				else
				{
					m_currentAmount = 0;
					--m_incrementAmount;
					m_placementDirection = eDirection::eUp;
				}
			}
			break;
			case eDirection::eUp:
			{
				if (m_currentAmount < m_incrementAmount)
				{
					++m_currentAmount;
					m_currentPlacementPosition.y -= tileSize.y;
				}
				else
				{
					if (m_incrementAmount == 7)
					{
						m_disabled = true;
					}
					m_currentAmount = 0;
					m_placementDirection = eDirection::eRight;
				}
			}
			break;
			}
		}

		if (tileManager.isTileOnPosition(eTileID::eBlank, Utilities::convertToGridPosition(m_currentPlacementPosition, tileSize)) ||
			tileManager.isTileOnPosition(eTileID::eBox, Utilities::convertToGridPosition(m_currentPlacementPosition, tileSize)))
		{
			tileManager.changeTile(eTileID::eWall, Utilities::convertToGridPosition(m_currentPlacementPosition, tileSize));

			sf::Packet packetToSend;
			packetToSend << eServerMessageType::eNewCollidableTile << m_currentPlacementPosition;
			server.broadcastMessage(packetToSend);
		}
	}
}