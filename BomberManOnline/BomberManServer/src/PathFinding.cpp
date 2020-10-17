#include "PathFinding.h"
#include "CollidableTile.h"
#include "Utilities.h"
#include "Server.h"
#include "TileID.h"
#include <math.h>
#include <queue>
#include <iostream>
#include <assert.h>
#include <math.h>
#include <assert.h>

constexpr size_t MAX_NEIGHBOURS = 4;

void getAdjacentPositions(sf::Vector2i position, const Server& server, sf::Vector2i ignorePosition, std::vector<sf::Vector2i>& positions);

void PathFinding::getNonCollidableAdjacentPositions(sf::Vector2i position, const Server& server, sf::Vector2i ignorePosition, std::vector<sf::Vector2i>& positions)
{
	for (sf::Vector2i direction : Utilities::getAllDirections())
	{
		sf::Vector2i adjacentPosition = position + direction;
		if (!server.getTileManager().isPositionCollidable(adjacentPosition) &&
			adjacentPosition != ignorePosition)
		{
			positions.push_back(adjacentPosition);
		}
	}
}

void getAdjacentPositions(sf::Vector2i position, const Server& server, sf::Vector2i ignorePosition, std::vector<sf::Vector2i>& positions)
{
	for (sf::Vector2i direction : Utilities::getAllDirections())
	{
		sf::Vector2i adjacentPosition = position + direction;
		if (adjacentPosition != ignorePosition)
		{
			positions.push_back(adjacentPosition);
		}
	}
}

//Graph Node
GraphNode::GraphNode()
	: cameFrom(),
	visited(false)
{}

GraphNode::GraphNode(sf::Vector2i cameFrom)
	: cameFrom(cameFrom),
	visited(true)
{}

sf::Vector2i GraphNode::getCameFrom() const
{
	return cameFrom;
}

bool GraphNode::isVisited() const
{
	return visited;
}

//Graph
Graph::Graph()
	: m_graph()
{}

void Graph::addToGraph(sf::Vector2i position, sf::Vector2i lastPosition, sf::Vector2i levelSize)
{
	assert(Utilities::isPositionInLevelBounds(position, levelSize));
	if (Utilities::isPositionInLevelBounds(position, levelSize) && !m_graph[position.y][position.x].isVisited())
	{
		m_graph[position.y][position.x] = GraphNode(lastPosition);
	}
}

bool Graph::isEmpty() const
{
	return m_graph.empty();
}

sf::Vector2i Graph::getPreviousPosition(sf::Vector2i position, sf::Vector2i levelSize) const
{
	assert(Utilities::isPositionInLevelBounds(position, levelSize) && m_graph[position.y][position.x].isVisited());
	if (Utilities::isPositionInLevelBounds(position, levelSize) && m_graph[position.y][position.x].isVisited())
	{
		return m_graph[position.y][position.x].getCameFrom();
	}
}

bool Graph::isPositionVisited(sf::Vector2i position, sf::Vector2i levelSize) const
{
	assert(Utilities::isPositionInLevelBounds(position, levelSize)); 
	if (Utilities::isPositionInLevelBounds(position, levelSize))
	{
		return m_graph[position.y][position.x].isVisited();
	}
}

void Graph::resetGraph(sf::Vector2i levelSize)
{
	if (m_graph.empty())
	{
		//Create
		m_graph.resize(levelSize.y);
		for (auto& row : m_graph)
		{
			std::vector<GraphNode> col;
			col.resize(levelSize.x);
			row = col;
		}
	}
	else
	{
		//Reset
		for (int y = 0; y < levelSize.y; y++)
		{
			for (int x = 0; x < levelSize.x; ++x)
			{
				m_graph[y][x] = GraphNode();
			}
		}
	}
}

//Path Finding
bool PathFinding::isPositionReachable(sf::Vector2f sourcePosition, sf::Vector2f targetPosition, const Server& server)
{
	if (sourcePosition == targetPosition)
	{
		return true;
	}
	
	sf::Vector2i levelSize = server.getLevelSize();
	reset(levelSize);

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i sourcePositionOnGrid = Utilities::convertToGridPosition(sourcePosition, tileSize);

	m_frontier.push(sourcePositionOnGrid);

	bool targetReachable = false;
	while (!targetReachable && !m_frontier.empty())
	{
		sf::Vector2i lastPosition = m_frontier.front();
		m_frontier.pop();

		getNonCollidableAdjacentPositions(lastPosition, server, sourcePositionOnGrid, m_adjacentPositions);
		for (sf::Vector2i adjacentPositions : m_adjacentPositions)
		{
			if (adjacentPositions == Utilities::convertToGridPosition(targetPosition, tileSize))
			{
				targetReachable = true;
				break;
			}
			else if (!m_graph.isPositionVisited(adjacentPositions, levelSize))
			{
				m_graph.addToGraph(adjacentPositions, lastPosition, levelSize);
				m_frontier.push(adjacentPositions);
			}
		}

		m_adjacentPositions.clear();
	}

	return targetReachable;
}

bool PathFinding::isPositionInRangeOfAllExplosions(sf::Vector2f sourcePosition, const Server& server)
{
	for (const auto& bomb : server.getBombs())
	{
		sf::Vector2i tileSize = server.getTileSize();
		sf::Vector2f bombPosition = bomb.getPosition();
		int explosionSize = bomb.getExplosionSize();

		if (bombPosition == sourcePosition)
		{
			return true;
		}

		for (sf::Vector2i direction : Utilities::getAllDirections())
		{
			for (int i = 1; i <= explosionSize; ++i)
			{
				sf::Vector2f explosionPosition = bombPosition + Utilities::scale(tileSize, direction, i);
				if (server.getTileManager().isPositionCollidable(Utilities::convertToGridPosition(explosionPosition, tileSize)))
				{
					break;
				}
				else if(explosionPosition == sourcePosition)
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool PathFinding::isPositionInRangeOfBombDetonation(sf::Vector2f sourcePosition, const BombServer& bomb, const Server& server)
{
	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2f bombPosition = Utilities::getClosestGridPosition(bomb.getPosition(), tileSize);
	int explosionSize = bomb.getExplosionSize();

	if (bombPosition == sourcePosition)
	{
		return true;
	}

	for (sf::Vector2i direction : Utilities::getAllDirections())
	{
		for (int i = 1; i <= explosionSize; ++i)
		{
			sf::Vector2f explosionPosition = bombPosition + Utilities::scale(tileSize, direction, explosionSize);
			if (server.getTileManager().isPositionCollidable(Utilities::convertToGridPosition(explosionPosition, tileSize)))
			{
				break;
			}
			else if (explosionPosition == sourcePosition)
			{
				return true;
			}
		}
	}

	return false;
}

void PathFinding::createGraph(sf::Vector2i levelSize)
{
	assert(m_graph.isEmpty());
	m_graph.resetGraph(levelSize);
	m_adjacentPositions.reserve(MAX_NEIGHBOURS);
}

void PathFinding::getPathToClosestBox(sf::Vector2f sourcePosition, std::vector<sf::Vector2f>& pathToTile, const Server& server, int maxBoxOptions)
{
	pathToTile.clear();
	sf::Vector2i levelSize = server.getLevelSize();
	reset(levelSize);
	
	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i sourcePositionOnGrid(Utilities::convertToGridPosition(sourcePosition, tileSize));

	m_frontier.push(sourcePositionOnGrid);

	std::vector<sf::Vector2i> boxSelection;
	boxSelection.reserve(static_cast<size_t>(maxBoxOptions));

	while (!m_frontier.empty() && boxSelection.size() < static_cast<size_t>(maxBoxOptions))
	{
		sf::Vector2i lastPosition = m_frontier.front();
		m_frontier.pop();

		getAdjacentPositions(lastPosition, server, sourcePositionOnGrid, m_adjacentPositions);
		for (sf::Vector2i adjacentPositions : m_adjacentPositions)
		{
			if (server.getTileManager().isTileOnPosition(eTileID::eWall, adjacentPositions) ||
				server.getTileManager().isTileOnPosition(eTileID::eBush, adjacentPositions))
			{
				continue;
			}

			if (!m_graph.isPositionVisited(adjacentPositions, server.getLevelSize()))
			{
				if (server.getTileManager().isTileOnPosition(eTileID::eBox, adjacentPositions) &&
					boxSelection.size() < static_cast<size_t>(maxBoxOptions))
				{
					boxSelection.push_back(adjacentPositions);
				}
				else
				{
					m_frontier.push(adjacentPositions);
				}

				m_graph.addToGraph(adjacentPositions, lastPosition, server.getLevelSize());
			}
		}

		m_adjacentPositions.clear();
	}

	if (!boxSelection.empty())
	{
		std::cout << boxSelection.size() << "\n";
		int randNumb = Utilities::getRandomNumber(0, static_cast<int>(boxSelection.size() - 1));
		sf::Vector2i targetPosition = m_graph.getPreviousPosition(boxSelection[randNumb], server.getLevelSize());

		getPathToVisitedTiles(sourcePositionOnGrid, targetPosition, pathToTile, server);
	}
}

void PathFinding::getPathToClosestPickUp(sf::Vector2f sourcePosition, std::vector<sf::Vector2f>& pathToTile, const Server& server, int range)
{
	pathToTile.clear();
	sf::Vector2i levelSize = server.getLevelSize();
	reset(levelSize);

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i sourcePositionOnGrid(Utilities::convertToGridPosition(sourcePosition, tileSize));

	m_frontier.push(sourcePositionOnGrid);

	bool pickUpFound = false;
	while (!pickUpFound && !m_frontier.empty())
	{
		sf::Vector2i lastPosition = m_frontier.front();
		m_frontier.pop();

		getNonCollidableAdjacentPositions(lastPosition, server, sourcePositionOnGrid, m_adjacentPositions);
		for (sf::Vector2i adjacentPositions : m_adjacentPositions)
		{
			if (!m_graph.isPositionVisited(adjacentPositions, levelSize))
			{
				m_graph.addToGraph(adjacentPositions, lastPosition, levelSize);
				if (getPathToVisitedTiles(sourcePositionOnGrid, adjacentPositions, server).size() > range)
				{
					continue;
				}
				m_frontier.push(adjacentPositions);

				if (server.isPickUpAtPosition(Utilities::convertToWorldPosition(adjacentPositions, tileSize)))
				{
					pickUpFound = true;
					getPathToVisitedTiles(sourcePositionOnGrid, adjacentPositions, pathToTile, server);
					break;
				}
			}
		}

		m_adjacentPositions.clear();
	}
}

void PathFinding::getPathToClosestSafePosition(sf::Vector2f sourcePosition, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	pathToTile.clear();
	sf::Vector2i levelSize = server.getLevelSize();
	reset(levelSize);

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i sourcePositionOnGrid(Utilities::convertToGridPosition(sourcePosition, tileSize));
	sf::Vector2i lastPosition;

	m_frontier.push(sourcePositionOnGrid);

	bool safePositionFound = false;
	while (!safePositionFound && !m_frontier.empty())
	{
		lastPosition = m_frontier.front();
		m_frontier.pop();

		getNonCollidableAdjacentPositions(lastPosition, server, sourcePositionOnGrid, m_adjacentPositions);
		for (sf::Vector2i adjacentPosition : m_adjacentPositions)
		{
			if (!m_graph.isPositionVisited(adjacentPosition, levelSize))
			{
				m_graph.addToGraph(adjacentPosition, lastPosition, levelSize);
				m_frontier.push(adjacentPosition);
				
				if (!isPositionInRangeOfAllExplosions(Utilities::convertToWorldPosition(adjacentPosition, tileSize), server))
				{
					lastPosition = adjacentPosition;
					safePositionFound = true;
					break;
				}
			}
		}

		m_adjacentPositions.clear();
	}

	if (safePositionFound)
	{
		getPathToVisitedTiles(sourcePositionOnGrid, lastPosition, pathToTile, server);
	}
}

void PathFinding::getPathToClosestSafePosition(sf::Vector2f sourcePosition, const BombServer& bomb, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	pathToTile.clear();
	sf::Vector2i levelSize = server.getLevelSize();
	reset(levelSize);

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i sourcePositionOnGrid(Utilities::convertToGridPosition(sourcePosition, tileSize));

	m_frontier.push(sourcePositionOnGrid);

	bool safePositionFound = false;
	while (!safePositionFound && !m_frontier.empty())
	{
		sf::Vector2i lastPosition = m_frontier.front();
		m_frontier.pop();

		getNonCollidableAdjacentPositions(lastPosition, server, sourcePositionOnGrid, m_adjacentPositions);
		for (const sf::Vector2i adjacentPositions : m_adjacentPositions)
		{
			if (!m_graph.isPositionVisited(adjacentPositions, levelSize))
			{
				m_graph.addToGraph(adjacentPositions, lastPosition, levelSize);
				m_frontier.push(adjacentPositions);
			}

			if (!isPositionInRangeOfBombDetonation(Utilities::convertToWorldPosition(adjacentPositions, tileSize), bomb, server))
			{
				getPathToVisitedTiles(sourcePositionOnGrid, adjacentPositions, pathToTile, server);
				safePositionFound = true;
				break;
			}
		}

		m_adjacentPositions.clear();
	}
}

void PathFinding::getPathToRandomLocalSafePosition(sf::Vector2f sourcePosition, std::vector<sf::Vector2f>& pathToTile, const Server& server, int maxPositionOptions)
{
	pathToTile.clear();
	sf::Vector2i levelSize = server.getLevelSize();
	reset(levelSize);

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i sourcePositionOnGrid(Utilities::convertToGridPosition(sourcePosition, tileSize));

	m_frontier.push(sourcePositionOnGrid);
	std::vector<sf::Vector2i> safePositions;
	safePositions.reserve(maxPositionOptions);
	
	while (safePositions.size() <= maxPositionOptions && !m_frontier.empty())
	{
		sf::Vector2i lastPosition = m_frontier.front();
		m_frontier.pop();

		getNonCollidableAdjacentPositions(lastPosition, server, sourcePositionOnGrid, m_adjacentPositions);
		for (sf::Vector2i adjacentPositions : m_adjacentPositions)
		{
			if (!m_graph.isPositionVisited(adjacentPositions, levelSize))
			{
				m_graph.addToGraph(adjacentPositions, lastPosition, levelSize);
				m_frontier.push(adjacentPositions);
			}

			if (!isPositionInRangeOfAllExplosions(Utilities::convertToWorldPosition(adjacentPositions, tileSize), server))
			{
				safePositions.push_back(adjacentPositions);
				if (safePositions.size() == maxPositionOptions)
				{
					break;
				}
			}
		}

		m_adjacentPositions.clear();
	}

	if (!safePositions.empty())
	{
		sf::Vector2i targetPosition = safePositions[Utilities::getRandomNumber(0, static_cast<int>(safePositions.size() - 1))];
		getPathToVisitedTiles(sourcePositionOnGrid, targetPosition, pathToTile, server);
	}
}

std::vector<sf::Vector2f> PathFinding::getPathToVisitedTiles(sf::Vector2i positionAtSource, sf::Vector2i targetPosition, const Server& server)
{
	std::vector<sf::Vector2f> pathToTile;
	pathToTile.push_back(Utilities::convertToWorldPosition(targetPosition, server.getTileSize()));

	sf::Vector2i position = targetPosition;
	while (position != positionAtSource)
	{
		position = m_graph.getPreviousPosition(position, server.getLevelSize());
		if (position != positionAtSource)
		{
			pathToTile.emplace_back(Utilities::convertToWorldPosition(position, server.getTileSize()));
		}
	}

	return pathToTile;
}

void PathFinding::getSafePathToTile(sf::Vector2f sourcePosition, sf::Vector2f targetPosition, const BombServer& bomb, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	pathToTile.clear();
	sf::Vector2i levelSize = server.getLevelSize();
	reset(levelSize);

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i sourcePositionOnGrid(Utilities::convertToGridPosition(sourcePosition, tileSize));
	sf::Vector2i targetPositionOnGrid(Utilities::convertToGridPosition(targetPosition, tileSize));

	m_frontier.push(sourcePositionOnGrid);

	sf::Vector2i lastPosition;
	bool pathCompleted = false;
	while (!pathCompleted && !m_frontier.empty())
	{
		lastPosition = m_frontier.front();
		m_frontier.pop();

		getNonCollidableAdjacentPositions(lastPosition, server, sourcePositionOnGrid, m_adjacentPositions);
		for (sf::Vector2i adjacentPositions : m_adjacentPositions)
		{
			if (!m_graph.isPositionVisited(adjacentPositions, levelSize) && 
				!isPositionInRangeOfBombDetonation(Utilities::convertToWorldPosition(adjacentPositions, tileSize), bomb, server))
			{
				m_graph.addToGraph(adjacentPositions, lastPosition, levelSize);
				m_frontier.push(adjacentPositions);
			}

			if (adjacentPositions == targetPositionOnGrid)
			{
				pathCompleted = true;
				break;
			}
		}

		m_adjacentPositions.clear();
	}
		
	if (pathCompleted)
	{
		getPathToVisitedTiles(sourcePositionOnGrid, lastPosition, pathToTile, server);
	}
}

void PathFinding::getSafePathToTile(sf::Vector2f sourcePosition, sf::Vector2f targetPosition, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	pathToTile.clear();
	sf::Vector2i levelSize = server.getLevelSize();
	reset(levelSize);

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i sourcePositionOnGrid(Utilities::convertToGridPosition(sourcePosition, tileSize));
	sf::Vector2i targetPositionOnGrid(Utilities::convertToGridPosition(targetPosition, tileSize));

	m_frontier.push(sourcePositionOnGrid);

	sf::Vector2i lastPosition;
	bool pathCompleted = false;
	while (!pathCompleted && !m_frontier.empty())
	{
		lastPosition = m_frontier.front();
		m_frontier.pop();

		getNonCollidableAdjacentPositions(lastPosition, server, sourcePositionOnGrid, m_adjacentPositions);
		for (sf::Vector2i adjacentPosition : m_adjacentPositions)
		{
			m_frontier.push(adjacentPosition);

			if (server.isBombAtPosition(Utilities::convertToWorldPosition(adjacentPosition, tileSize)))
			{
				continue;
			}
			else if (!isPositionInRangeOfAllExplosions(Utilities::convertToWorldPosition(adjacentPosition, tileSize), server) &&
				!m_graph.isPositionVisited(adjacentPosition, levelSize))
			{
				m_graph.addToGraph(adjacentPosition, lastPosition, levelSize);
			}

			if (adjacentPosition == targetPositionOnGrid)
			{
				lastPosition = adjacentPosition;
				pathCompleted = true;
				break;
			}
		}

		m_adjacentPositions.clear();
	}

	if (pathCompleted)
	{
		getPathToVisitedTiles(sourcePositionOnGrid, lastPosition, pathToTile, server);
	}
}

void PathFinding::getNonCollidableAdjacentPositions(sf::Vector2f position, const Server& server, std::vector<sf::Vector2f>& positions)
{
	sf::Vector2i sourcePositionOnGrid = Utilities::convertToGridPosition(position, server.getTileSize());
	for (sf::Vector2i direction : Utilities::getAllDirections())
	{
		sf::Vector2i adjacentPosition = sourcePositionOnGrid + direction;
		if (!server.getTileManager().isPositionCollidable(adjacentPosition))
		{
			positions.push_back(Utilities::convertToWorldPosition(adjacentPosition, server.getTileSize()));
		}
	}
}

sf::Vector2f PathFinding::getFurthestNonCollidablePosition(sf::Vector2f sourcePosition, eDirection direction, const Server& server, int maxDistance) const
{
	assert(direction != eDirection::eNone);
	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2f furthestPosition;
	switch (direction)
	{
	case eDirection::eLeft:
	{
		for (int x = sourcePosition.x; x >= sourcePosition.x - (tileSize.x * maxDistance);)
		{
			furthestPosition = sf::Vector2f(x, sourcePosition.y);
			if (!server.getTileManager().isPositionCollidable(Utilities::convertToGridPosition(furthestPosition, tileSize)))
			{
				x -= tileSize.x;
			}
			else
			{
				furthestPosition = sf::Vector2f(x + tileSize.x, sourcePosition.y);
				break;
			}
		}
	}
	break;
	case eDirection::eRight:
	{
		for (int x = sourcePosition.x; x < sourcePosition.x + (tileSize.x * maxDistance);)
		{
			furthestPosition = sf::Vector2f(x, sourcePosition.y);
			if (!server.getTileManager().isPositionCollidable(Utilities::convertToGridPosition(furthestPosition, tileSize)))
			{
				x += tileSize.x;
			}
			else
			{
				furthestPosition = sf::Vector2f(x - tileSize.x, sourcePosition.y);
				break;
			}
		}
	}
	break;
	case eDirection::eUp:
	{
		for (int y = sourcePosition.y; y >= sourcePosition.y - (tileSize.y * maxDistance);)
		{
			furthestPosition = sf::Vector2f(sourcePosition.x, y);
			if (!server.getTileManager().isPositionCollidable(Utilities::convertToGridPosition(furthestPosition, tileSize)))
			{
				y -= tileSize.y;
			}
			else
			{
				furthestPosition = sf::Vector2f(sourcePosition.x, y + tileSize.y);
				break;
			}
		}
	}
	break;
	case eDirection::eDown:
	{
		for (int y = sourcePosition.y; y < sourcePosition.y + (tileSize.y * maxDistance);)
		{
			furthestPosition = sf::Vector2f(sourcePosition.x, y);
			if (!server.getTileManager().isPositionCollidable(Utilities::convertToGridPosition(furthestPosition, tileSize)))
			{
				y += tileSize.y;
			}
			else
			{
				furthestPosition = sf::Vector2f(sourcePosition.x, y - tileSize.y);
				break;
			}
		}
	}
	break;
	}

	return furthestPosition;
}

std::vector<sf::Vector2f> PathFinding::getPathToTile(sf::Vector2f sourcePosition, sf::Vector2f targetPosition, const Server& server) 
{
	std::vector<sf::Vector2f> pathToTile;

	assert(isPositionReachable(sourcePosition, targetPosition, server));
	if (isPositionReachable(sourcePosition, targetPosition, server))
	{
		sf::Vector2i levelSize = server.getLevelSize();
		reset(levelSize);

		sf::Vector2i tileSize = server.getTileSize();
		sf::Vector2i sourcePositionOnGrid(Utilities::convertToGridPosition(sourcePosition, tileSize));

		m_frontier.push(sourcePositionOnGrid);

		sf::Vector2i lastPosition;
		bool destinationFound = false;
		while (!m_frontier.empty() && !destinationFound)
		{
			lastPosition = m_frontier.front();
			m_frontier.pop();

			getNonCollidableAdjacentPositions(lastPosition, server, sourcePositionOnGrid, m_adjacentPositions);
			for (sf::Vector2i adjacentPositions : m_adjacentPositions)
			{
				if (!m_graph.isPositionVisited(adjacentPositions, levelSize))
				{
					m_graph.addToGraph(adjacentPositions, lastPosition, levelSize);
					m_frontier.push(adjacentPositions);
				}

				if (Utilities::convertToWorldPosition(adjacentPositions, tileSize) == targetPosition)
				{
					destinationFound = true;
					lastPosition = adjacentPositions;
					break;
				}
			}

			m_adjacentPositions.clear();
		}

		if (destinationFound)
		{
			pathToTile = getPathToVisitedTiles(sourcePositionOnGrid, lastPosition, server);
		}
	}

	return pathToTile;
}

void PathFinding::getPathToTile(sf::Vector2f sourcePosition, sf::Vector2f targetPosition, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	pathToTile.clear();

	assert(isPositionReachable(sourcePosition, targetPosition, server));
	if (isPositionReachable(sourcePosition, targetPosition, server))
	{
		sf::Vector2i levelSize = server.getLevelSize();
		reset(levelSize);

		sf::Vector2i tileSize = server.getTileSize();
		sf::Vector2i sourcePositionOnGrid(Utilities::convertToGridPosition(sourcePosition, tileSize));

		m_frontier.push(sourcePositionOnGrid);

		sf::Vector2i lastPosition;
		bool destinationFound = false;
		while (!m_frontier.empty() && !destinationFound)
		{
			lastPosition = m_frontier.front();
			m_frontier.pop();

			getNonCollidableAdjacentPositions(lastPosition, server, sourcePositionOnGrid, m_adjacentPositions);
			for (sf::Vector2i adjacentPositions : m_adjacentPositions)
			{
				if (!m_graph.isPositionVisited(adjacentPositions, levelSize))
				{
					m_graph.addToGraph(adjacentPositions, lastPosition, levelSize);
					m_frontier.push(adjacentPositions);
				}

				if (Utilities::convertToWorldPosition(adjacentPositions, tileSize) == targetPosition)
				{
					destinationFound = true;
					lastPosition = adjacentPositions;
					break;
				}
			}

			m_adjacentPositions.clear();
		}

		if (destinationFound)
		{
			getPathToVisitedTiles(sourcePositionOnGrid, lastPosition, pathToTile, server);
		}
	}
}

void PathFinding::getPathToVisitedTiles(sf::Vector2i sourcePosition, sf::Vector2i targetPosition, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	pathToTile.push_back(Utilities::convertToWorldPosition(targetPosition, server.getTileSize()));

	sf::Vector2i position = sf::Vector2i(targetPosition);
	while (position != sourcePosition)
	{
		position = m_graph.getPreviousPosition(position, server.getLevelSize());
		if (position != sourcePosition)
		{
			pathToTile.push_back(Utilities::convertToWorldPosition(position, server.getTileSize()));
		}
	}
}

void PathFinding::reset(sf::Vector2i levelSize)
{
	m_graph.resetGraph(levelSize);
	m_adjacentPositions.clear();
	std::queue<sf::Vector2i> emptyFrontier;
	m_frontier.swap(emptyFrontier);
}