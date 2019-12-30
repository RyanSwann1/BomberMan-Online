#include "PathFinding.h"
#include "CollidableTile.h"
#include "Utilities.h"
#include "Server.h"
#include <math.h>
#include <queue>
#include <iostream>
#include <assert.h>
#include <math.h>
#include <assert.h>

constexpr size_t MAX_NEIGHBOURS = 4;
constexpr size_t MAX_BOX_SELECTION = 5;
constexpr int FURTHEST_NON_COLLIDABLE_POSITION = 4;

void getAdjacentPositions(sf::Vector2i position, const Server& server, sf::Vector2i ignorePosition, std::vector<sf::Vector2i>& positions);

void PathFinding::getNonCollidableAdjacentPositions(sf::Vector2i position, const Server& server, sf::Vector2i ignorePosition, std::vector<sf::Vector2i>& positions)
{
	for (int x = position.x - 1; x <= position.x + 1; x += 2)
	{
		if (x >= 0 && x < server.getLevelSize().x && server.getCollidableTile({ x, position.y }) != eCollidableTile::eWall
			&& server.getCollidableTile({ x, position.y }) != eCollidableTile::eBox &&
			sf::Vector2i(x, position.y) != ignorePosition)
		{
			positions.emplace_back(x, position.y);
		}
	}

	for (int y = position.y - 1; y <= position.y + 1; y += 2)
	{
		if (y >= 0 && y < server.getLevelSize().y && server.getCollidableTile({ position.x, y }) != eCollidableTile::eWall
			&& server.getCollidableTile({ position.x, y }) != eCollidableTile::eBox && 
			sf::Vector2i(position.x, y) != ignorePosition)
		{
			positions.emplace_back(position.x, y);
		}

	}
}

void getAdjacentPositions(sf::Vector2i position, const Server& server, sf::Vector2i ignorePosition, std::vector<sf::Vector2i>& positions)
{
	for (int x = position.x - 1; x <= position.x + 1; x += 2)
	{
		if (x >= 0 && x < server.getLevelSize().x && sf::Vector2i(x, position.y) != ignorePosition)
		{
			positions.emplace_back(x, position.y);
		}
	}

	for (int y = position.y - 1; y <= position.y + 1; y += 2)
	{
		if (y >= 0 && y < server.getLevelSize().y && sf::Vector2i(position.x, y) != ignorePosition)
		{
			positions.emplace_back(position.x, y);
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
	assert(position.x >= 0 && position.x < levelSize.x && position.y >= 0 && position.y < levelSize.y);
	if (position.x >= 0 && position.x < levelSize.x && position.y >= 0 || position.y < levelSize.y && !m_graph[position.y][position.y].isVisited())
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
	assert(position.x >= 0 && position.x < levelSize.x && position.y >= 0 && position.y < levelSize.y && m_graph[position.y][position.x].isVisited());
	if (position.x >= 0 && position.x < levelSize.x && position.y >= 0 && position.y < levelSize.y && m_graph[position.y][position.x].isVisited())
	{
		return m_graph[position.y][position.x].getCameFrom();
	}
}

bool Graph::isPositionVisited(sf::Vector2i position, sf::Vector2i levelSize) const
{
	assert(position.x >= 0 && position.x < levelSize.x && position.y >= 0 && position.y < levelSize.y);
	if (position.x >= 0 && position.x < levelSize.x && position.y >= 0 && position.y < levelSize.y)
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
		for (sf::Vector2i neighbourPosition : m_adjacentPositions)
		{
			if (neighbourPosition == Utilities::convertToGridPosition(targetPosition, tileSize))
			{
				targetReachable = true;
				break;
			}
			else if (!m_graph.isPositionVisited(neighbourPosition, levelSize))
			{
				m_graph.addToGraph(neighbourPosition, lastPosition, levelSize);
				m_frontier.push(neighbourPosition);
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
		sf::Vector2f bombPosition = Utilities::getClosestGridPosition(bomb.getPosition(), tileSize);
		int explosionSize = bomb.getExplosionSize();
		sf::Vector2f explosionPosition;

		//Up
		for (int y = bombPosition.y; y >= bombPosition.y - (tileSize.y * explosionSize); y -= tileSize.y)
		{
			explosionPosition = sf::Vector2f(bombPosition.x, y);
			if (server.getCollidableTile(Utilities::convertToGridPosition(explosionPosition, tileSize)) != eCollidableTile::eNonCollidable)
			{
				break;
			}
			else if (explosionPosition == sourcePosition)
			{
				return true;
			}
		}

		//Down
		for (int y = bombPosition.y; y <= bombPosition.y + (tileSize.y * explosionSize); y += tileSize.y)
		{
			explosionPosition = sf::Vector2f(bombPosition.x, y);
			if (server.getCollidableTile(Utilities::convertToGridPosition(explosionPosition, tileSize)) != eCollidableTile::eNonCollidable)
			{
				break;
			}
			else if (explosionPosition == sourcePosition)
			{
				return true;
			}
		}

		//Left
		for (int x = bombPosition.x; x >= bombPosition.x - (tileSize.x * explosionSize); x -= tileSize.x)
		{
			explosionPosition = sf::Vector2f(x, bombPosition.y);
			if (server.getCollidableTile(Utilities::convertToGridPosition(explosionPosition, tileSize)) != eCollidableTile::eNonCollidable)
			{
				break;
			}
			else if (explosionPosition == sourcePosition)
			{
				return true;
			}
		}

		//Right
		for (int x = bombPosition.x; x <= bombPosition.x + (tileSize.x * explosionSize); x += tileSize.x)
		{
			explosionPosition = sf::Vector2f(x, bombPosition.y);
			if (server.getCollidableTile(Utilities::convertToGridPosition(explosionPosition, tileSize)) != eCollidableTile::eNonCollidable)
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

bool PathFinding::isPositionInRangeOfBombDetonation(sf::Vector2f sourcePosition, const BombServer& bomb, const Server& server)
{
	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2f bombPosition = Utilities::getClosestGridPosition(bomb.getPosition(), tileSize);
	int explosionSize = bomb.getExplosionSize();
	sf::Vector2f explosionPosition;

	//left
	for (int x = bombPosition.x; x >= bombPosition.x - (tileSize.x * explosionSize); x -= tileSize.x)
	{
		explosionPosition = sf::Vector2f(x, bombPosition.y);
		if (server.getCollidableTile(Utilities::convertToGridPosition(explosionPosition, tileSize)) != eCollidableTile::eNonCollidable)
		{
			break;
		}
		else if (explosionPosition == sourcePosition)
		{
			return true;
		}
	}

	//Right
	for (int x = bombPosition.x; x <= bombPosition.x + (tileSize.x * explosionSize); x += tileSize.x)
	{
		explosionPosition = sf::Vector2f(x, bombPosition.y);
		if (server.getCollidableTile(Utilities::convertToGridPosition(explosionPosition, tileSize)) != eCollidableTile::eNonCollidable)
		{
			break;
		}
		else if (explosionPosition == sourcePosition)
		{
			return true;
		}
	}

	//Up
	for (int y = bombPosition.y; y >= bombPosition.y - (tileSize.y * explosionSize); y -= tileSize.y)
	{
		explosionPosition = sf::Vector2f(bombPosition.x, y);
		if (server.getCollidableTile(Utilities::convertToGridPosition(explosionPosition, tileSize)) != eCollidableTile::eNonCollidable)
		{
			break;
		}
		else if (explosionPosition == sourcePosition)
		{
			return true;
		}
	}

	//Down
	for (int y = bombPosition.y; y <= bombPosition.y + (tileSize.y * explosionSize); y += tileSize.y)
	{
		explosionPosition = sf::Vector2f(bombPosition.x, y);
		if (server.getCollidableTile(Utilities::convertToGridPosition(explosionPosition, tileSize)) != eCollidableTile::eNonCollidable)
		{
			break;
		}
		else if (explosionPosition == sourcePosition)
		{
			return true;
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

void PathFinding::getPathToClosestBox(sf::Vector2f sourcePosition, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	pathToTile.clear();
	sf::Vector2i levelSize = server.getLevelSize();
	reset(levelSize);
	
	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i sourcePositionOnGrid(Utilities::convertToGridPosition(sourcePosition, tileSize));

	m_frontier.push(sourcePositionOnGrid);

	std::vector<sf::Vector2i> boxSelection;
	boxSelection.reserve(MAX_BOX_SELECTION);

	while (!m_frontier.empty() && boxSelection.size() < MAX_BOX_SELECTION)
	{
		sf::Vector2i lastPosition = m_frontier.front();
		m_frontier.pop();

		getAdjacentPositions(lastPosition, server, sourcePositionOnGrid, m_adjacentPositions);
		for (sf::Vector2i neighbourPosition : m_adjacentPositions)
		{
			if (server.getCollidableTile(neighbourPosition) == eCollidableTile::eWall)
			{
				continue;
			}

			if (!m_graph.isPositionVisited(neighbourPosition, server.getLevelSize()))
			{
				if (server.getCollidableTile(neighbourPosition) == eCollidableTile::eBox &&
					boxSelection.size() < MAX_BOX_SELECTION)
				{
					boxSelection.push_back(neighbourPosition);
				}
				else
				{
					m_frontier.push(neighbourPosition);
				}

				m_graph.addToGraph(neighbourPosition, lastPosition, server.getLevelSize());
			}
		}

		m_adjacentPositions.clear();
	}

	if (!boxSelection.empty())
	{
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
		for (sf::Vector2i neighbourPosition : m_adjacentPositions)
		{
			if (!m_graph.isPositionVisited(neighbourPosition, levelSize))
			{
				m_graph.addToGraph(neighbourPosition, lastPosition, levelSize);
				if (getPathToVisitedTiles(sourcePositionOnGrid, neighbourPosition, server).size() > range)
				{
					continue;
				}
				m_frontier.push(neighbourPosition);

				if (server.isPickUpAtPosition(Utilities::convertToWorldPosition(neighbourPosition, tileSize)))
				{
					pickUpFound = true;
					getPathToVisitedTiles(sourcePositionOnGrid, neighbourPosition, pathToTile, server);
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
		for (const sf::Vector2i neighbourPosition : m_adjacentPositions)
		{
			if (!m_graph.isPositionVisited(neighbourPosition, levelSize))
			{
				m_graph.addToGraph(neighbourPosition, lastPosition, levelSize);
				m_frontier.push(neighbourPosition);
			}

			if (!isPositionInRangeOfBombDetonation(Utilities::convertToWorldPosition(neighbourPosition, tileSize), bomb, server))
			{
				getPathToVisitedTiles(sourcePositionOnGrid, neighbourPosition, pathToTile, server);
				safePositionFound = true;
				break;
			}
		}

		m_adjacentPositions.clear();
	}
}

void PathFinding::getPathToLocalSafePosition(sf::Vector2f sourcePosition, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	pathToTile.clear();
	sf::Vector2i levelSize = server.getLevelSize();
	reset(levelSize);

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i sourcePositionOnGrid(Utilities::convertToGridPosition(sourcePosition, tileSize));

	m_frontier.push(sourcePositionOnGrid);
	std::vector<sf::Vector2i> safePositions;
	safePositions.reserve(5);
	
	while (safePositions.size() <= 5 && !m_frontier.empty())
	{
		sf::Vector2i lastPosition = m_frontier.front();
		m_frontier.pop();

		getNonCollidableAdjacentPositions(lastPosition, server, sourcePositionOnGrid, m_adjacentPositions);
		for (sf::Vector2i neighbourPosition : m_adjacentPositions)
		{
			if (!m_graph.isPositionVisited(neighbourPosition, levelSize))
			{
				m_graph.addToGraph(neighbourPosition, lastPosition, levelSize);
				m_frontier.push(neighbourPosition);
			}

			if (!isPositionInRangeOfAllExplosions(Utilities::convertToWorldPosition(neighbourPosition, tileSize), server))
			{
				safePositions.push_back(neighbourPosition);
				if (safePositions.size() == 5)
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
		for (sf::Vector2i neighbourPosition : m_adjacentPositions)
		{
			if (server.isBombAtPosition(Utilities::convertToWorldPosition(neighbourPosition, tileSize)))
			{
				continue;
			}
			else if (!isPositionInRangeOfBombDetonation(Utilities::convertToWorldPosition(neighbourPosition, tileSize), bomb, server) &&
				!m_graph.isPositionVisited(neighbourPosition, levelSize))
			{
				m_graph.addToGraph(neighbourPosition, lastPosition, levelSize);
				m_frontier.push(neighbourPosition);
			}

			if (neighbourPosition == targetPositionOnGrid)
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

	for (int x = sourcePositionOnGrid.x - 1; x <= sourcePositionOnGrid.x + 1; x += 2)
	{
		if (x >= 0 && x < server.getLevelSize().x && server.getCollidableTile({ x, sourcePositionOnGrid.y }) != eCollidableTile::eWall
			&& server.getCollidableTile({ x, sourcePositionOnGrid.y }) != eCollidableTile::eBox)
		{
			positions.push_back(Utilities::convertToWorldPosition({ x, sourcePositionOnGrid.y }, server.getTileSize()));
		}
	}

	for (int y = sourcePositionOnGrid.y - 1; y <= sourcePositionOnGrid.y + 1; y += 2)
	{
		if (y >= 0 && y < server.getLevelSize().y && server.getCollidableTile({ sourcePositionOnGrid.x, y }) != eCollidableTile::eWall
			&& server.getCollidableTile({ sourcePositionOnGrid.x, y }) != eCollidableTile::eBox)
		{
			positions.push_back(Utilities::convertToWorldPosition({ sourcePositionOnGrid.x, y }, server.getTileSize()));
		}
	}
}

sf::Vector2f PathFinding::getFurthestNonCollidablePosition(sf::Vector2f sourcePosition, eDirection direction, const Server& server) const
{
	assert(direction != eDirection::eNone);
	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2f furthestPosition;
	switch (direction)
	{
	case eDirection::eLeft:
	{
		for (int x = sourcePosition.x; x >= sourcePosition.x - (tileSize.x * FURTHEST_NON_COLLIDABLE_POSITION);)
		{
			furthestPosition = sf::Vector2f(x, sourcePosition.y);
			if (server.getCollidableTile(Utilities::convertToGridPosition(furthestPosition, tileSize)) == eCollidableTile::eNonCollidable)
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
		for (int x = sourcePosition.x; x < sourcePosition.x + (tileSize.x * FURTHEST_NON_COLLIDABLE_POSITION);)
		{
			furthestPosition = sf::Vector2f(x, sourcePosition.y);
			if (server.getCollidableTile(Utilities::convertToGridPosition(furthestPosition, tileSize)) == eCollidableTile::eNonCollidable)
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
		for (int y = sourcePosition.y; y >= sourcePosition.y - (tileSize.y * FURTHEST_NON_COLLIDABLE_POSITION);)
		{
			furthestPosition = sf::Vector2f(sourcePosition.x, y);
			if (server.getCollidableTile(Utilities::convertToGridPosition(furthestPosition, tileSize)) == eCollidableTile::eNonCollidable)
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
		for (int y = sourcePosition.y; y < sourcePosition.y + (tileSize.y * FURTHEST_NON_COLLIDABLE_POSITION);)
		{
			furthestPosition = sf::Vector2f(sourcePosition.x, y);
			if (server.getCollidableTile(Utilities::convertToGridPosition(furthestPosition, tileSize)) == eCollidableTile::eNonCollidable)
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
			for (sf::Vector2i neighbourPosition : m_adjacentPositions)
			{
				if (!m_graph.isPositionVisited(neighbourPosition, levelSize))
				{
					m_graph.addToGraph(neighbourPosition, lastPosition, levelSize);
					m_frontier.push(neighbourPosition);
				}

				if (Utilities::convertToWorldPosition(neighbourPosition, tileSize) == targetPosition)
				{
					destinationFound = true;
					lastPosition = neighbourPosition;
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
			for (sf::Vector2i neighbourPosition : m_adjacentPositions)
			{
				if (!m_graph.isPositionVisited(neighbourPosition, levelSize))
				{
					m_graph.addToGraph(neighbourPosition, lastPosition, levelSize);
					m_frontier.push(neighbourPosition);
				}

				if (Utilities::convertToWorldPosition(neighbourPosition, tileSize) == targetPosition)
				{
					destinationFound = true;
					lastPosition = neighbourPosition;
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