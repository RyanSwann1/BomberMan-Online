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

void getNonCollidableNeighbouringPoints(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours, const Server& server);
void getNeighbouringPoints(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours, const Server& server);

void getNonCollidableNeighbouringPoints(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours, const Server& server)
{
	for (int x = position.x - 1; x <= position.x + 1; x += 2)
	{
		if (x >= 0 && x < server.getLevelSize().x && server.getCollisionLayer()[position.y][x] != eCollidableTile::eWall
			&& server.getCollisionLayer()[position.y][x] != eCollidableTile::eBox)
		{
			neighbours.emplace_back(x, position.y);
		}
	}

	for (int y = position.y - 1; y <= position.y + 1; y += 2)
	{
		if (y >= 0 && y < server.getLevelSize().y && server.getCollisionLayer()[y][position.x] != eCollidableTile::eWall
			&& server.getCollisionLayer()[y][position.x] != eCollidableTile::eBox)
		{
			neighbours.emplace_back(position.x, y);
		}
	}
}

void getNeighbouringPoints(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours, const Server& server)
{
	for (int x = position.x - 1; x <= position.x + 1; x += 2)
	{
		if (x >= 0 && x < server.getLevelSize().x && server.getCollisionLayer()[position.y][x] != eCollidableTile::eWall)
		{
			neighbours.emplace_back(x, position.y);
		}
	}

	for (int y = position.y - 1; y <= position.y + 1; y += 2)
	{
		if (y >= 0 && y < server.getLevelSize().y && server.getCollisionLayer()[y][position.x] != eCollidableTile::eWall)
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

void PathFinding::resetGraph(sf::Vector2i levelSize, std::vector<std::vector<GraphNode>>& graph)
{
	for (int y = 0; y < levelSize.y; y++)
	{
		for (int x = 0; x < levelSize.x; ++x)
		{
			graph[y][x] = GraphNode(); 
		}
	}
}

void PathFinding::addToGraph(std::vector<std::vector<GraphNode>>& graph, sf::Vector2i position, sf::Vector2i levelSize)
{
	assert(position.x >= 0 || position.x < levelSize.x || position.y >= 0 || position.y < levelSize.y || !graph[position.y][position.y].visited);
	if()
	graph[position.y][position.x] = GraphNode(position);
}

GraphNode PathFinding::getGraphNode(sf::Vector2i position, sf::Vector2i levelSize, const std::vector<std::vector<GraphNode>>& graph)
{
	assert(position.x < 0 || position.x > levelSize.x || position.y < 0 || position.y > levelSize.y);
	if (position.x < 0 || position.x > levelSize.x || position.y < 0 || position.y > levelSize.y)
	{
		return graph[position.y][position.x];
	}
}

void PathFinding::createGraph(sf::Vector2i levelSize)
{
	m_graph.resize(levelSize.y);
	for (auto& row : m_graph)
	{
		std::vector<GraphNode> col;
		col.resize(levelSize.x);
		row = col;
	}
}

sf::Vector2f PathFinding::getPositionClosestToTarget(sf::Vector2f source, sf::Vector2f target, const Server& server)
{
	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);
	sf::Vector2i tileSize = server.getTileSize();
	
	sf::Vector2i roundedTargetPosition(static_cast<int>(target.x / tileSize.x), static_cast<int>(target.y / tileSize.y));
	sf::Vector2i closestPosition = sf::Vector2i(static_cast<int>(source.x / tileSize.x), static_cast<int>(source.y / tileSize.y));
	getNeighbouringPoints(closestPosition, neighbours, server);

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
	sf::Vector2i levelSize = server.getLevelSize();
	sf::Vector2i tileSize = server.getTileSize();
	const auto& collisionLayer = server.getCollisionLayer();

	resetGraph(levelSize, m_graph);

	std::queue<sf::Vector2i> frontier;
	frontier.push(sf::Vector2i(static_cast<int>(source.x / tileSize.x), static_cast<int>(source.y / tileSize.y)));

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	sf::Vector2i lastPosition = sf::Vector2i(static_cast<int>(source.x / tileSize.x), static_cast<int>(source.y / tileSize.y));
	while (!frontier.empty())
	{
		lastPosition = frontier.front();
		frontier.pop();
		getNeighbouringPoints(lastPosition, neighbours, server);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			//Target found
			if (neighbourPosition == sf::Vector2i(static_cast<int>(target.x / tileSize.x), static_cast<int>(target.y / tileSize.y)))
			{
				return true;
			}
			else if (collisionLayer[neighbourPosition.y][neighbourPosition.x] == eCollidableTile::eNonCollidable &&
				!m_graph[neighbourPosition.y][neighbourPosition.x].visited)
			{
				m_graph[neighbourPosition.y][neighbourPosition.x] = GraphNode(lastPosition);
				frontier.push(neighbourPosition);
			}
		}
	}

	//Target Unreachable
	return false;
}

void PathFinding::getPathToTile(sf::Vector2f source, sf::Vector2f destination, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	pathToTile.clear();
	resetGraph(server.getLevelSize(), m_graph);

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i positionAtSource(source.x / tileSize.x, source.y / tileSize.y);
	sf::Vector2i positionAtDestination(destination.x / tileSize.x, destination.y / tileSize.y);
	
	std::queue<sf::Vector2i> frontier;
	frontier.push(positionAtSource);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	bool reachedDestination = false;
	while (!reachedDestination && !frontier.empty())
	{
		sf::Vector2i lastPosition = frontier.front();
		frontier.pop();

		getNonCollidableNeighbouringPoints(lastPosition, neighbours, server);
		for (sf::Vector2i neighbourPosition : neighbours)
		{		
			if (Utilities::distance(neighbourPosition, positionAtDestination) < Utilities::distance(lastPosition, positionAtDestination))
			{
				m_graph[neighbourPosition.y][neighbourPosition.x] = GraphNode(lastPosition);
				pathToTile.emplace_back(neighbourPosition.x * tileSize.x, neighbourPosition.y * tileSize.y);
				frontier.push(neighbourPosition);
				break;
			}	
		}
		
		if (!pathToTile.empty() && pathToTile.back() == sf::Vector2f(static_cast<float>(destination.x * tileSize.x), 
			static_cast<float>(destination.y * tileSize.y)))
		{
			reachedDestination = true;
		}

		neighbours.clear();
	}
}

void PathFinding::getPathToClosestBox(sf::Vector2f source, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	sf::Vector2i levelSize = server.getLevelSize();
	sf::Vector2i tileSize = server.getTileSize();

	pathToTile.clear();
	sf::Vector2i positionAtSource(source.x / tileSize.x, source.y / tileSize.y);
	resetGraph(levelSize, m_graph);

	std::queue<sf::Vector2i> frontier;
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
		getNeighbouringPoints(lastPosition, neighbours, server);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			if (server.getCollisionLayer()[neighbourPosition.y][neighbourPosition.x] == eCollidableTile::eBox)
			{
				if (boxSelection.size() < MAX_BOX_SELECTION)
				{
					m_graph[neighbourPosition.y][neighbourPosition.x] = GraphNode(lastPosition);
					boxSelection.push_back(neighbourPosition);
				}
			}
			else
			{
				if (!m_graph[neighbourPosition.y][neighbourPosition.x].visited)
				{
					m_graph[neighbourPosition.y][neighbourPosition.x] = GraphNode(lastPosition);
					frontier.push(neighbourPosition);
				}
			}
		}

		neighbours.clear();
	}

	if (!boxSelection.empty())
	{
		int randNumb = Utilities::getRandomNumber(0, static_cast<int>(boxSelection.size() - 1));
		lastPosition = m_graph[boxSelection[randNumb].y][boxSelection[randNumb].x].cameFrom;

		sf::Vector2i comeFrom = lastPosition;
		pathToTile.emplace_back(comeFrom.x * tileSize.x, comeFrom.y * tileSize.y);
		
		bool pathCompleted = false;
		while (!pathCompleted)
		{
			comeFrom = m_graph[comeFrom.y][comeFrom.x].cameFrom;
			if (comeFrom != positionAtSource)
			{
				pathToTile.emplace_back(comeFrom.x * tileSize.x, comeFrom.y * tileSize.y);
			}
			else
			{
				pathCompleted = true;
			}
		}
	}
}

void PathFinding::getPathToClosestPickUp(sf::Vector2f source, std::vector<sf::Vector2f>& pathToTile, const Server& server, int range)
{
	pathToTile.clear();
	resetGraph(server.getLevelSize(), m_graph);

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

		getNonCollidableNeighbouringPoints(lastPosition, neighbours, server);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			if (getPathToTile(source, sf::Vector2f(neighbourPosition.x * tileSize.x, neighbourPosition.y * tileSize.y), server).size() 
				> range)
			{
				continue;
			}

			if (!m_graph[neighbourPosition.y][neighbourPosition.x].visited)
			{
				addToGraph
				m_graph[neighbourPosition.y][neighbourPosition.x] = GraphNode(lastPosition);
				frontier.push(neighbourPosition);
				std::cout << neighbourPosition.x << " " << neighbourPosition.y << "\n";
				//std::cout << frontier.size() << "\n";
			}

			auto gameObject = std::find_if(server.getGameObjects().cbegin(), server.getGameObjects().cend(), [neighbourPosition, tileSize](const auto& gameObject)
			{
				if (gameObject.getType() == eGameObjectType::eMovementPickUp)
				{
					return gameObject.getPosition() == sf::Vector2f(neighbourPosition.x * tileSize, neighbourPosition.y * tileSize);
				}
			});
			if (gameObject != server.getGameObjects().cend())
			{
				searchForPickUp = false;
				getPathToTile(source, sf::Vector2f(neighbourPosition.x * tileSize.x, neighbourPosition.y * tileSize.y), pathToTile, server);
			}
		}

		neighbours.clear();
	}
}

void PathFinding::getPathToClosestSafePosition(sf::Vector2f source, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	pathToTile.clear();
	resetGraph(server.getLevelSize(), m_graph);

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

		getNonCollidableNeighbouringPoints(lastPosition, neighbours, server);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			if (neighbourPosition.x != positionAtSource.x && neighbourPosition.y != positionAtSource.y)
			{
				pathToTile.emplace_back(neighbourPosition.x * tileSize.x, neighbourPosition.y * tileSize.y);
				pathToTile.emplace_back(lastPosition.x * tileSize.x, lastPosition.y * tileSize.y);
				safePositionFound = true;
				sf::Vector2i comeFrom = lastPosition;
				bool pathCompleted = false;
				while (!pathCompleted)
				{
					comeFrom = m_graph[comeFrom.y][comeFrom.x].cameFrom;
					if (comeFrom != positionAtSource)
					{
						pathToTile.emplace_back(comeFrom.x * tileSize.x, comeFrom.y * tileSize.y);
					}
					else
					{
						pathCompleted = true;
					}
				}
				break;
			}
			else
			{
				if (!m_graph[neighbourPosition.y][neighbourPosition.x].visited)
				{
					m_graph[neighbourPosition.y][neighbourPosition.x] = GraphNode(lastPosition);
					frontier.push(neighbourPosition);
				}
			}
		}

		neighbours.clear();
	}
}

std::vector<sf::Vector2i> PathFinding::getPathToTile(sf::Vector2f source, sf::Vector2f destination, const Server& server)
{
	resetGraph(server.getLevelSize(), m_graph);

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i positionAtSource(source.x / tileSize.x, source.y / tileSize.y);

	std::queue<sf::Vector2i> frontier;
	frontier.push(positionAtSource);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	sf::Vector2i positionAtDestination(destination.x, destination.y);
	std::vector<sf::Vector2i> pathToTile;
	bool reachedDestination = false;
	while (!reachedDestination && !frontier.empty())
	{
		sf::Vector2i lastPosition = frontier.front();
		frontier.pop();

		getNonCollidableNeighbouringPoints(lastPosition, neighbours, server);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			if (Utilities::distance(neighbourPosition, positionAtDestination) < Utilities::distance(lastPosition, positionAtDestination))
			{
				m_graph[neighbourPosition.y][neighbourPosition.x] = GraphNode(lastPosition);
				pathToTile.emplace_back(neighbourPosition.x * tileSize.x, neighbourPosition.y * tileSize.y);
				frontier.push(neighbourPosition);
				break;
			}			
		}

		if (!pathToTile.empty() && pathToTile.back() == sf::Vector2i(static_cast<float>(destination.x * tileSize.x), static_cast<float>(destination.y * tileSize.y)))
		{
			reachedDestination = true;
		}

		neighbours.clear();
	}

	return pathToTile;
}