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
	assert(position.x >= 0 || position.x < levelSize.x || position.y >= 0 || position.y < levelSize.y || !m_graph[position.y][position.y].isVisited());
	if (position.x >= 0 || position.x < levelSize.x || position.y >= 0 || position.y < levelSize.y || !m_graph[position.y][position.y].isVisited())
	{
		m_graph[position.y][position.x] = GraphNode(lastPosition);
	}
}

const std::vector<std::vector<GraphNode>>& Graph::getGraph() const
{
	return m_graph;
}

GraphNode Graph::getGraphNode(sf::Vector2i position, sf::Vector2i levelSize)
{
	assert((position.x >= 0 || position.x < levelSize.x || position.y >= 0 || position.y < levelSize.y));
	if (position.x >= 0 || position.x < levelSize.x || position.y >= 0 || position.y < levelSize.y)
	{
		return m_graph[position.y][position.x];
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
sf::Vector2f PathFinding::getPositionClosestToTarget(sf::Vector2f source, sf::Vector2f target, const Server& server)
{
	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i positionAtSource = sf::Vector2i(source.x / tileSize.x, source.y / tileSize.y);
	
	sf::Vector2i roundedTargetPosition(static_cast<int>(target.x / tileSize.x), static_cast<int>(target.y / tileSize.y));
	sf::Vector2i closestPosition = sf::Vector2i(static_cast<int>(source.x / tileSize.x), static_cast<int>(source.y / tileSize.y));

	getNonCollidableNeighbouringPoints(positionAtSource, neighbours, server, positionAtSource);
	for (sf::Vector2i neighbour : neighbours)
	{
		if (Utilities::distance(neighbour, roundedTargetPosition) < Utilities::distance(closestPosition, roundedTargetPosition))
		{
			closestPosition = neighbour;
		}
	}

	return sf::Vector2f(closestPosition.x * tileSize.x, closestPosition.y * tileSize.y);
}

bool PathFinding::isPositionReachable(sf::Vector2f source, sf::Vector2f target, const Server& server)
{
	m_graph.resetGraph(server.getLevelSize());

	std::queue<sf::Vector2i> frontier;
	sf::Vector2i tileSize = server.getTileSize();
	frontier.push(sf::Vector2i(static_cast<int>(source.x / tileSize.x), static_cast<int>(source.y / tileSize.y)));

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	sf::Vector2i lastPosition = sf::Vector2i(static_cast<int>(source.x / tileSize.x), static_cast<int>(source.y / tileSize.y));
	while (!frontier.empty())
	{
		lastPosition = frontier.front();
		frontier.pop();

		getNonCollidableNeighbouringPoints(lastPosition, neighbours, server, sf::Vector2i(source.x / tileSize.x, source.y / tileSize.y));
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			//Target Reachable
			if (neighbourPosition == sf::Vector2i(static_cast<int>(target.x / tileSize.x), static_cast<int>(target.y / tileSize.y)))
			{
				return true;
			}
			else if (!m_graph.getGraphNode(neighbourPosition, server.getLevelSize()).isVisited())
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

void PathFinding::createGraph(sf::Vector2i levelSize)
{
	assert(m_graph.getGraph().empty());
	m_graph.resetGraph(levelSize);
}

void PathFinding::getPathToClosestBox(sf::Vector2f source, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	pathToTile.clear();
	m_graph.resetGraph(server.getLevelSize());

	std::queue<sf::Vector2i> frontier;
	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i positionAtSource(source.x / tileSize.x, source.y / tileSize.y);
	frontier.push(positionAtSource);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	std::vector<sf::Vector2i> boxSelection;
	boxSelection.reserve(MAX_BOX_SELECTION);
	sf::Vector2i lastPosition;

	while (!frontier.empty() && boxSelection.size() < MAX_BOX_SELECTION)
	{
		lastPosition = frontier.front();
		frontier.pop();

		getNeighbouringPoints(lastPosition, neighbours, server, positionAtSource);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			if (server.getCollidableTile(neighbourPosition) == eCollidableTile::eWall)
			{
				continue;
			}

			if (!m_graph.getGraphNode(neighbourPosition, server.getLevelSize()).isVisited())
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
		sf::Vector2i position = m_graph.getGraphNode(boxSelection[randNumb], server.getLevelSize()).getCameFrom();
		pathToTile.emplace_back(position.x * tileSize.x, position.y * tileSize.y);
		
		while (position != positionAtSource)
		{
			position = m_graph.getGraphNode(position, server.getLevelSize()).getCameFrom();
			pathToTile.emplace_back(position.x * tileSize.x, position.y * tileSize.y);
		}
	}
}

void PathFinding::getPathToClosestPickUp(sf::Vector2f source, std::vector<sf::Vector2f>& pathToTile, const Server& server, int range)
{
	pathToTile.clear();
	m_graph.resetGraph(server.getLevelSize());

	sf::Vector2i tileSize = server.getTileSize();
	std::queue<sf::Vector2i> frontier;
	sf::Vector2i positionAtSource(source.x / tileSize.x, source.y / tileSize.y);
	frontier.push(positionAtSource);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	bool searchForPickUp = true;
	while (searchForPickUp && !frontier.empty())
	{
		sf::Vector2i lastPosition = frontier.front();
		frontier.pop();

		getNonCollidableNeighbouringPoints(lastPosition, neighbours, server, positionAtSource);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			if (!m_graph.getGraphNode(neighbourPosition, server.getLevelSize()).isVisited())
			{
				m_graph.addToGraph(neighbourPosition, lastPosition, server.getLevelSize());
				frontier.push(neighbourPosition);
			}

			if (getPathToTile(neighbourPosition, lastPosition, server, positionAtSource).size() 
				> range)
			{
				continue;
			}
			
			for (const GameObject& gameObject : server.getGameObjects())
			{
				if (gameObject.getPosition() == sf::Vector2f(neighbourPosition.x * tileSize.x, neighbourPosition.y * tileSize.y)
					&& (gameObject.getType() == eGameObjectType::eMovementPickUp ||
					gameObject.getType() == eGameObjectType::eExtraBombPickUp ||
					gameObject.getType() == eGameObjectType::eBiggerExplosionPickUp))
				{
					searchForPickUp = false;
					getPathToTile(neighbourPosition, server, positionAtSource, pathToTile);
					break;
				}
			}
		}

		neighbours.clear();
	}
}

void PathFinding::getPathToClosestSafePosition(sf::Vector2f source, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	pathToTile.clear();
	m_graph.resetGraph(server.getLevelSize());

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i positionAtSource(source.x / tileSize.x, source.y / tileSize.y);
	
	std::queue<sf::Vector2i> frontier;
	frontier.push(positionAtSource);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	bool safePositionFound = false;
	while (!frontier.empty() && !safePositionFound)
	{
		sf::Vector2i lastPosition = frontier.front();
		frontier.pop();

		getNonCollidableNeighbouringPoints(lastPosition, neighbours, server, positionAtSource);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			if (!m_graph.getGraphNode(neighbourPosition, server.getLevelSize()).isVisited())
			{
				m_graph.addToGraph(neighbourPosition, lastPosition, server.getLevelSize());
				frontier.push(neighbourPosition);
			}

			if (neighbourPosition.x != positionAtSource.x && neighbourPosition.y != positionAtSource.y)
			{
				getPathToTile(neighbourPosition, server, positionAtSource, pathToTile);
				safePositionFound = true;
				break;
			}
		}

		neighbours.clear();
	}
}

std::vector<sf::Vector2f> PathFinding::getPathToTile(sf::Vector2i targetPosition, sf::Vector2i lastPosition, const Server& server, sf::Vector2i positionAtSource)
{
	//No path
	if (lastPosition == positionAtSource)
	{
		return std::vector<sf::Vector2f>();
	}

	std::vector<sf::Vector2f> pathToTile;
	sf::Vector2i tileSize = server.getTileSize();
	pathToTile.emplace_back(targetPosition.x* tileSize.x, targetPosition.y* tileSize.y);
	pathToTile.emplace_back(lastPosition.x* tileSize.x, lastPosition.y* tileSize.y);

	sf::Vector2i position = lastPosition;
	while (position != positionAtSource)
	{
		position = m_graph.getGraphNode(position, server.getLevelSize()).getCameFrom();
		pathToTile.emplace_back(position.x * tileSize.x, position.y * tileSize.y);
	}

	return pathToTile;
}

void PathFinding::getPathToTile(sf::Vector2i targetPosition, sf::Vector2i lastPosition, const Server& server, sf::Vector2i positionAtSource, std::vector<sf::Vector2f>& pathToTile)
{
	//No path
	if (lastPosition == positionAtSource)
	{
		return;
	}

	sf::Vector2i tileSize = server.getTileSize();
	pathToTile.emplace_back(targetPosition.x * tileSize.x, targetPosition.y * tileSize.y);
	pathToTile.emplace_back(lastPosition.x * tileSize.x, lastPosition.y * tileSize.y);

	sf::Vector2i position = lastPosition;
	while (position != positionAtSource)
	{
		position = m_graph.getGraphNode(position, server.getLevelSize()).getCameFrom();
		pathToTile.emplace_back(position.x * tileSize.x, position.y * tileSize.y);
	}
}

void PathFinding::getPathToTile(sf::Vector2i targetPosition, const Server& server, sf::Vector2i positionAtSource, std::vector<sf::Vector2f>& pathToTile)
{
	sf::Vector2i tileSize = server.getTileSize();
	pathToTile.emplace_back(targetPosition.x * tileSize.x, targetPosition.y * tileSize.y);

	sf::Vector2i position = sf::Vector2i(targetPosition);
	while (position != positionAtSource)
	{
		position = m_graph.getGraphNode(position, server.getLevelSize()).getCameFrom();
		pathToTile.emplace_back(position.x * tileSize.x, position.y * tileSize.y);
	}
}
