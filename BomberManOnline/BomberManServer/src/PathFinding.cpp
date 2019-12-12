#include "PathFinding.h"
#include "CollidableTile.h"
#include "Utilities.h"
#include "Server.h"
#include <math.h>
#include <queue>
#include <iostream>
#include <assert.h>

constexpr size_t MAX_NEIGHBOURS = 4;
constexpr size_t MAX_BOX_SELECTION = 5;
constexpr int FURTHEST_NON_COLLIDABLE_POSITION = 4;

void getNonCollidableNeighbouringPoints(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours, const Server& server, sf::Vector2i ignorePosition);
void getNeighbouringPoints(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours, const Server& server, sf::Vector2i ignorePosition);

void getNonCollidableNeighbouringPoints(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours, const Server& server, sf::Vector2i ignorePosition)
{
	for (int x = position.x - 1; x <= position.x + 1; x += 2)
	{
		if (x >= 0 && x < server.getLevelSize().x && server.getCollidableTile({ x, position.y }) != eCollidableTile::eWall
			&& server.getCollidableTile({ x, position.y }) != eCollidableTile::eBox &&
			sf::Vector2i(x, position.y) != ignorePosition)
		{
			neighbours.emplace_back(x, position.y);
		}
	}

	for (int y = position.y - 1; y <= position.y + 1; y += 2)
	{
		if (y >= 0 && y < server.getLevelSize().y && server.getCollidableTile({ position.x, y }) != eCollidableTile::eWall
			&& server.getCollidableTile({ position.x, y }) != eCollidableTile::eBox && 
			sf::Vector2i(position.x, y) != ignorePosition)
		{
			neighbours.emplace_back(position.x, y);
		}
	}
}

void getNeighbouringPoints(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours, const Server& server, sf::Vector2i ignorePosition)
{
	for (int x = position.x - 1; x <= position.x + 1; x += 2)
	{
		if (x >= 0 && x < server.getLevelSize().x && sf::Vector2i(x, position.y) != ignorePosition)
		{
			neighbours.emplace_back(x, position.y);
		}
	}

	for (int y = position.y - 1; y <= position.y + 1; y += 2)
	{
		if (y >= 0 && y < server.getLevelSize().y && sf::Vector2i(position.x, y) != ignorePosition)
		{
			neighbours.emplace_back(position.x, y);
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

//Path Finding
void PathFinding::getPositionClosestToTarget(sf::Vector2f sourcePosition, sf::Vector2f targetPosition, const Server& server, std::vector<sf::Vector2f>& pathToTile)
{
	pathToTile.clear();

	std::vector<sf::Vector2f> newPathToTile(getPathToTile(sourcePosition, targetPosition, server));
	assert(!newPathToTile.empty());
	if (!newPathToTile.empty())
	{
		pathToTile.push_back(newPathToTile.back());
	}
}

bool PathFinding::isPositionReachable(sf::Vector2f sourcePosition, sf::Vector2f targetPosition, const Server& server)
{
	assert(sourcePosition != targetPosition);
	//if (sourcePosition == targetPosition)
	//{
	//	return true;
	//}

	m_graph.resetGraph(server.getLevelSize());

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i sourcePositionOnGrid = Utilities::convertToGridPosition(sourcePosition, tileSize);

	std::queue<sf::Vector2i> frontier;
	frontier.push(sourcePositionOnGrid);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	while (!frontier.empty())
	{
		sf::Vector2i lastPosition = frontier.front();
		frontier.pop();

		getNonCollidableNeighbouringPoints(lastPosition, neighbours, server, sourcePositionOnGrid);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			//Target Reachable
			if (neighbourPosition == Utilities::convertToGridPosition(targetPosition, tileSize))
			{
				return true;
			}
			else if (!m_graph.isPositionVisited(neighbourPosition, server.getLevelSize()))
			{
				m_graph.addToGraph(neighbourPosition, lastPosition, server.getLevelSize());
				frontier.push(neighbourPosition);
			}
		}

		neighbours.clear();
	}

	//Target Unreachable
	return false;
}

bool PathFinding::isPositionInRangeOfAllExplosions(sf::Vector2f sourcePosition, const Server& server)
{
	for (const auto& bomb : server.getBombs())
	{
		sf::Vector2i tileSize = server.getTileSize();
		sf::Vector2f bombPosition = bomb.getPosition();
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

bool PathFinding::isPositionInRangeOfExplosion(sf::Vector2f sourcePosition, const BombServer& bomb, const Server& server)
{
	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2f bombPosition = bomb.getPosition();
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
}

void PathFinding::getPathToClosestBox(sf::Vector2f sourcePosition, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	pathToTile.clear();
	m_graph.resetGraph(server.getLevelSize());

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i sourcePositionOnGrid(Utilities::convertToGridPosition(sourcePosition, tileSize));

	std::queue<sf::Vector2i> frontier;
	frontier.push(sourcePositionOnGrid);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	std::vector<sf::Vector2i> boxSelection;
	boxSelection.reserve(MAX_BOX_SELECTION);

	while (!frontier.empty() && boxSelection.size() < MAX_BOX_SELECTION)
	{
		sf::Vector2i lastPosition = frontier.front();
		frontier.pop();

		getNeighbouringPoints(lastPosition, neighbours, server, sourcePositionOnGrid);
		for (sf::Vector2i neighbourPosition : neighbours)
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
					frontier.push(neighbourPosition);
				}

				m_graph.addToGraph(neighbourPosition, lastPosition, server.getLevelSize());
			}
		}

		neighbours.clear();
	}

	if (!boxSelection.empty())
	{
		int randNumb = Utilities::getRandomNumber(0, static_cast<int>(boxSelection.size() - 1));
		sf::Vector2i position = m_graph.getPreviousPosition(boxSelection[randNumb], server.getLevelSize());
		pathToTile.emplace_back(Utilities::convertToWorldPosition(position, tileSize));
		
		while (position != sourcePositionOnGrid)
		{
			position = m_graph.getPreviousPosition(position, server.getLevelSize());
			pathToTile.emplace_back(Utilities::convertToWorldPosition(position, tileSize));
		}
	}
}

void PathFinding::getPathToClosestPickUp(sf::Vector2f sourcePosition, std::vector<sf::Vector2f>& pathToTile, const Server& server, int range)
{
	pathToTile.clear();
	m_graph.resetGraph(server.getLevelSize());

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i sourcePositionOnGrid(Utilities::convertToGridPosition(sourcePosition, tileSize));
	
	std::queue<sf::Vector2i> frontier;
	frontier.push(sourcePositionOnGrid);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	bool searchForPickUp = true;
	while (searchForPickUp && !frontier.empty())
	{
		sf::Vector2i lastPosition = frontier.front();
		frontier.pop();

		getNonCollidableNeighbouringPoints(lastPosition, neighbours, server, sourcePositionOnGrid);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			if (!m_graph.isPositionVisited(neighbourPosition, server.getLevelSize()))
			{
				m_graph.addToGraph(neighbourPosition, lastPosition, server.getLevelSize());
				frontier.push(neighbourPosition);
			}

			if (getPathToTile(sourcePositionOnGrid, neighbourPosition, server).size() > range)
			{
				continue;
			}
			
			for (const GameObject& gameObject : server.getGameObjects())
			{
				if (gameObject.getPosition() == sf::Vector2f(Utilities::convertToWorldPosition(neighbourPosition, tileSize))
					&& (gameObject.getType() == eGameObjectType::eMovementPickUp ||
					gameObject.getType() == eGameObjectType::eExtraBombPickUp ||
					gameObject.getType() == eGameObjectType::eBiggerExplosionPickUp))
				{
					searchForPickUp = false;
					getPathToTile(sourcePositionOnGrid, neighbourPosition, pathToTile, server);
					break;
				}
			}
		}

		neighbours.clear();
	}
}

void PathFinding::getPathToClosestSafePosition(sf::Vector2f sourcePosition, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	pathToTile.clear();
	m_graph.resetGraph(server.getLevelSize());

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i sourcePositionOnGrid(Utilities::convertToGridPosition(sourcePosition, tileSize));
	
	std::queue<sf::Vector2i> frontier;
	frontier.push(sourcePositionOnGrid);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	bool safePositionFound = false;
	while (!frontier.empty() && !safePositionFound)
	{
		sf::Vector2i lastPosition = frontier.front();
		frontier.pop();

		getNonCollidableNeighbouringPoints(lastPosition, neighbours, server, sourcePositionOnGrid);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			if (!m_graph.isPositionVisited(neighbourPosition, server.getLevelSize()))
			{
				m_graph.addToGraph(neighbourPosition, lastPosition, server.getLevelSize());
				frontier.push(neighbourPosition);
			}

			if (!isPositionInRangeOfAllExplosions(Utilities::convertToWorldPosition(neighbourPosition, tileSize), server))
			{
				getPathToTile(sourcePositionOnGrid, neighbourPosition, pathToTile, server);
				safePositionFound = true;
				break;
			}
		}

		neighbours.clear();
	}
}

void PathFinding::getPathToClosestSafePosition(sf::Vector2f sourcePosition, const BombServer& bomb, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	pathToTile.clear();
	m_graph.resetGraph(server.getLevelSize());

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i sourcePositionOnGrid(Utilities::convertToGridPosition(sourcePosition, tileSize));

	std::queue<sf::Vector2i> frontier;
	frontier.push(sourcePositionOnGrid);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	bool safePositionFound = false;
	while (!safePositionFound && !frontier.empty())
	{
		sf::Vector2i lastPosition = frontier.front();
		frontier.pop();

		getNonCollidableNeighbouringPoints(lastPosition, neighbours, server, sourcePositionOnGrid);
		for (const sf::Vector2i neighbourPosition : neighbours)
		{
			if (!m_graph.isPositionVisited(neighbourPosition, server.getLevelSize()))
			{
				m_graph.addToGraph(neighbourPosition, lastPosition, server.getLevelSize());
				frontier.push(neighbourPosition);
			}
			
			if (!isPositionInRangeOfExplosion(Utilities::convertToWorldPosition(neighbourPosition, tileSize), bomb, server))
			{
				getPathToTile(sourcePositionOnGrid, neighbourPosition, pathToTile, server);
				safePositionFound = true;
				break;
			}
		}

		neighbours.clear();
	}
}

std::vector<sf::Vector2f> PathFinding::getPathToTile(sf::Vector2i positionAtSource, sf::Vector2i targetPosition, const Server& server)
{
	sf::Vector2i tileSize = server.getTileSize();
	std::vector<sf::Vector2f> pathToTile;
	pathToTile.emplace_back(Utilities::convertToWorldPosition(targetPosition, tileSize));

	sf::Vector2i position = targetPosition;
	while (position != positionAtSource)
	{
		position = m_graph.getPreviousPosition(position, server.getLevelSize());
		pathToTile.emplace_back(Utilities::convertToWorldPosition(position, tileSize));
	}

	return pathToTile;
}

void PathFinding::getSafePathToTile(sf::Vector2f sourcePosition, sf::Vector2f targetPosition, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	pathToTile.clear();
	m_graph.resetGraph(server.getLevelSize());

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i sourcePositionOnGrid(Utilities::convertToGridPosition(sourcePosition, tileSize));

	std::queue<sf::Vector2i> frontier;
	frontier.push(sourcePositionOnGrid);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	bool pathCompleted = false;
	while (!pathCompleted && !frontier.empty())
	{
		sf::Vector2i lastPosition = frontier.front();
		frontier.pop();

		getNonCollidableNeighbouringPoints(lastPosition, neighbours, server, sourcePositionOnGrid);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			if (Utilities::isPositionNeighbourToPosition(Utilities::convertToWorldPosition(neighbourPosition, tileSize), targetPosition, tileSize) &&
				!m_graph.isPositionVisited(neighbourPosition, server.getLevelSize()))
			{
				m_graph.addToGraph(neighbourPosition, lastPosition, server.getLevelSize());
				frontier.push(neighbourPosition);
			}
			else if (!isPositionInRangeOfAllExplosions(Utilities::convertToWorldPosition(neighbourPosition, tileSize), server) &&
				!m_graph.isPositionVisited(neighbourPosition, server.getLevelSize()))
			{
				m_graph.addToGraph(neighbourPosition, lastPosition, server.getLevelSize());
				frontier.push(neighbourPosition);
			}

			if (targetPosition == Utilities::convertToWorldPosition(neighbourPosition, tileSize))
			{
				sf::Vector2f position = Utilities::convertToWorldPosition(neighbourPosition, tileSize);
				pathToTile.emplace_back(position);
				while (position != targetPosition)
				{
					auto pos = m_graph.getPreviousPosition(Utilities::convertToGridPosition(position, tileSize), server.getLevelSize());
					position = sf::Vector2f(pos.x * tileSize.x, pos.y * tileSize.y);
					pathToTile.push_back(position);
				}

				pathCompleted = true;
				break;
			}
		}

		neighbours.clear();
	}
}

sf::Vector2f PathFinding::getFurthestNonCollidablePosition(sf::Vector2f sourcePosition, eDirection direction, const Server& server) const
{
	sf::Vector2i levelSize = server.getLevelSize();
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
		m_graph.resetGraph(server.getLevelSize());

		sf::Vector2i tileSize = server.getTileSize();
		sf::Vector2i sourcePositionOnGrid(Utilities::convertToGridPosition(sourcePosition, tileSize));

		std::queue<sf::Vector2i> frontier;
		frontier.push(sourcePositionOnGrid);

		std::vector<sf::Vector2i> neighbouringPositions;
		neighbouringPositions.reserve(MAX_NEIGHBOURS);

		bool destinationFound = false;
		while (!frontier.empty() && !destinationFound)
		{
			sf::Vector2i lastPosition = frontier.front();
			frontier.pop();

			getNonCollidableNeighbouringPoints(lastPosition, neighbouringPositions, server, sourcePositionOnGrid);
			for (sf::Vector2i neighbourPosition : neighbouringPositions)
			{
				if (!m_graph.isPositionVisited(neighbourPosition, server.getLevelSize()))
				{
					m_graph.addToGraph(neighbourPosition, lastPosition, server.getLevelSize());
					frontier.push(neighbourPosition);
				}

				if (Utilities::convertToWorldPosition(neighbourPosition, tileSize) == targetPosition)
				{
					destinationFound = true;
					break;
				}
			}
		}

		if (destinationFound)
		{
			sf::Vector2f position(Utilities::convertToWorldPosition(frontier.back(), tileSize));
			pathToTile.push_back(position);
			bool pathCompleted = false;
			while (!pathCompleted)
			{
				sf::Vector2i previousPosition = m_graph.getPreviousPosition(Utilities::convertToGridPosition(position, tileSize), server.getLevelSize());
				position = sf::Vector2f(Utilities::convertToWorldPosition(previousPosition, tileSize));
				if (position != sourcePosition)
				{
					pathToTile.push_back(position);
				}
				else
				{
					pathCompleted = true;
				}
			}
		}
	}

	return pathToTile;
}

void PathFinding::getPathToTile(sf::Vector2i sourcePosition, sf::Vector2i targetPosition, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	sf::Vector2i tileSize = server.getTileSize();
	pathToTile.emplace_back(Utilities::convertToWorldPosition(targetPosition, tileSize));

	sf::Vector2i position = sf::Vector2i(targetPosition);
	while (position != sourcePosition)
	{
		position = m_graph.getPreviousPosition(position, server.getLevelSize());
		pathToTile.emplace_back(Utilities::convertToWorldPosition(position, tileSize));
	}
}
