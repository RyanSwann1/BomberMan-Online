#include "PathFinding.h"
#include "CollidableTile.h"
#include "Utilities.h"
#include "Server.h"
#include <math.h>
#include <queue>

constexpr size_t MAX_NEIGHBOURS = 4;
constexpr size_t MAX_BOX_SELECTION = 5;

//Graph Node
GraphNode::GraphNode()
	: cameFrom(),
	visited(false)
{}

GraphNode::GraphNode(sf::Vector2i cameFrom)
	: cameFrom(cameFrom),
	visited(true)
{}

void resetGraph(sf::Vector2i levelSize, std::vector<std::vector<GraphNode>>& graph);

void getNeighbouringPoints(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours, const Server& server);

void resetGraph(sf::Vector2i levelSize, std::vector<std::vector<GraphNode>>& graph)
{
	for (int y = 0; y < levelSize.y; y++)
	{
		for (int x = 0; x < levelSize.x; ++x)
		{
			graph[y][x] = GraphNode();
		}
	}
}

void getNeighbouringPoints(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours, const Server& server)
{
	sf::Vector2i levelSize = server.getLevelSize();
	const auto& collisionLayer = server.getCollisionLayer();

	for (int x = position.x - 1; x <= position.x + 1; x += 2)
	{
		if (x >= 0 && x < levelSize.x && collisionLayer[position.y][x] != eCollidableTile::eWall)
		{
			neighbours.emplace_back(x, position.y);
		}
	}

	for (int y = position.y - 1; y <= position.y + 1; y += 2)
	{
		if (y >= 0 && y < levelSize.y && collisionLayer[y][position.x] != eCollidableTile::eWall)
		{
			neighbours.emplace_back(position.x, y);
		}
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

bool PathFinding::isPositionReachableWithinRange(sf::Vector2f source, sf::Vector2f target, const Server& server, int range)
{
	return false;
}

void PathFinding::getPathToTile(sf::Vector2f source, sf::Vector2f destination, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	sf::Vector2i levelSize = server.getLevelSize();
	sf::Vector2i tileSize = server.getTileSize();
	const auto& collisionLayer = server.getCollisionLayer();

	pathToTile.clear();
	sf::Vector2i positionAtSource(source.x / tileSize.x, source.y / tileSize.y);
	sf::Vector2i positionAtDestination(destination.x / tileSize.x, destination.y / tileSize.y);
	resetGraph(levelSize, m_graph);

	std::queue<sf::Vector2i> frontier;
	frontier.push(positionAtSource);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	bool reachedDestination = false;
	while (!reachedDestination)
	{
		sf::Vector2i lastPosition = frontier.front();
		frontier.pop();
		getNeighbouringPoints(lastPosition, neighbours, server);
		for (auto& neighbourPosition : neighbours)
		{
			if (collisionLayer[neighbourPosition.y][neighbourPosition.x] == eCollidableTile::eBox)
			{
				continue;
			}

			if (pathToTile.empty())
			{
				if (Utilities::distance(neighbourPosition, positionAtDestination) < Utilities::distance(positionAtSource, positionAtDestination))
				{
					m_graph[neighbourPosition.y][neighbourPosition.x] = GraphNode(lastPosition);
					pathToTile.emplace_back(neighbourPosition.x * tileSize.x, neighbourPosition.y * tileSize.y);
					frontier.push(neighbourPosition);
					break;
				}
			}
			else
			{
				if (Utilities::distance(neighbourPosition, positionAtDestination) < Utilities::distance(lastPosition, positionAtDestination))
				{
					m_graph[neighbourPosition.y][neighbourPosition.x] = GraphNode(lastPosition);
					pathToTile.emplace_back(neighbourPosition.x * tileSize.x, neighbourPosition.y * tileSize.y);
					frontier.push(neighbourPosition);
					break;
				}
			}
		}

		if (pathToTile.back() == sf::Vector2f(static_cast<float>(destination.x * tileSize.x), 
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
	sf::Vector2i levelSize = server.getLevelSize();
	sf::Vector2i tileSize = server.getTileSize();
	const auto& collisionLayer = server.getCollisionLayer();

	pathToTile.clear();
	sf::Vector2i positionAtSource(source.x / tileSize.x, source.y / tileSize.y);
	resetGraph(levelSize, m_graph);

	std::queue<sf::Vector2i> frontier;
	frontier.push(positionAtSource);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	bool searchForPickUp = true;
	while (searchForPickUp || !frontier.empty())
	{
		sf::Vector2i lastPosition = frontier.front();
		frontier.pop();

		getNeighbouringPoints(lastPosition, neighbours, server);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			if (server.getCollisionLayer()[neighbourPosition.y][neighbourPosition.x] == eCollidableTile::eBox ||
				getPathToTile(source, sf::Vector2f(neighbourPosition.x, neighbourPosition.y), server).size() > range)
			{
				continue;
			}

			frontier.push(neighbourPosition);

			sf::Vector2f position(neighbourPosition.x, neighbourPosition.y);
			auto cIter = std::find_if(server.getGameObjects().cbegin(), server.getGameObjects().cend(), [position](const auto& gameObject)
				{
					return gameObject.getPosition() == position && gameObject.getType() == eGameObjectType::eMovementPickUp;
				});

			if (cIter != server.getGameObjects().cend())
			{
				searchForPickUp = false;
				getPathToTile(source, sf::Vector2f(neighbourPosition.x, neighbourPosition.y), pathToTile, server);
				return;
			}
		}
	}
}

void PathFinding::getPathToClosestSafePosition(sf::Vector2f source, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	sf::Vector2i levelSize = server.getLevelSize();
	sf::Vector2i tileSize = server.getTileSize();
	const auto& collisionLayer = server.getCollisionLayer();

	pathToTile.clear();
	sf::Vector2i positionAtSource(source.x / tileSize.x, source.y / tileSize.y);
	resetGraph(levelSize, m_graph);

	std::queue<sf::Vector2i> frontier;
	frontier.push(positionAtSource);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	bool safePositionFound = false;
	while (!frontier.empty())
	{
		sf::Vector2i lastPosition = frontier.front();
		frontier.pop();
		getNeighbouringPoints(lastPosition, neighbours, server);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			if (collisionLayer[neighbourPosition.y][neighbourPosition.x] == eCollidableTile::eBox)
			{
				continue;
			}

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

		if (safePositionFound)
		{
			break;
		}
	}
}

std::vector<sf::Vector2i> PathFinding::getPathToTile(sf::Vector2f source, sf::Vector2f destination, const Server& server)
{
	sf::Vector2i levelSize = server.getLevelSize();
	sf::Vector2i tileSize = server.getTileSize();
	const auto& collisionLayer = server.getCollisionLayer();

	sf::Vector2i positionAtSource(source.x / tileSize.x, source.y / tileSize.y);
	sf::Vector2i positionAtDestination(destination.x / tileSize.x, destination.y / tileSize.y);
	resetGraph(levelSize, m_graph);

	std::queue<sf::Vector2i> frontier;
	frontier.push(positionAtSource);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);
	
	std::vector<sf::Vector2i> pathToTile;
	bool reachedDestination = false;
	while (!reachedDestination)
	{
		sf::Vector2i lastPosition = frontier.front();
		frontier.pop();
		getNeighbouringPoints(lastPosition, neighbours, server);
		for (auto& neighbourPosition : neighbours)
		{
			if (collisionLayer[neighbourPosition.y][neighbourPosition.x] == eCollidableTile::eBox)
			{
				continue;
			}

			if (pathToTile.empty())
			{
				if (Utilities::distance(neighbourPosition, positionAtDestination) < Utilities::distance(positionAtSource, positionAtDestination))
				{
					m_graph[neighbourPosition.y][neighbourPosition.x] = GraphNode(lastPosition);
					pathToTile.emplace_back(neighbourPosition.x * tileSize.x, neighbourPosition.y * tileSize.y);
					frontier.push(neighbourPosition);
					break;
				}
			}
			else
			{
				if (Utilities::distance(neighbourPosition, positionAtDestination) < Utilities::distance(lastPosition, positionAtDestination))
				{
					m_graph[neighbourPosition.y][neighbourPosition.x] = GraphNode(lastPosition);
					pathToTile.emplace_back(neighbourPosition.x * tileSize.x, neighbourPosition.y * tileSize.y);
					frontier.push(neighbourPosition);
					break;
				}
			}
		}

		if (pathToTile.back() == sf::Vector2i(static_cast<float>(destination.x * tileSize.x),
			static_cast<float>(destination.y * tileSize.y)))
		{
			reachedDestination = true;
		}

		neighbours.clear();
	}

	return pathToTile;
}