#include "PathFinding.h"
#include "CollidableTile.h"
#include "Utilities.h"
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

void getNeighbouringPoints(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours,
	const std::vector<std::vector<eCollidableTile>>& collisionLayer, sf::Vector2i levelSize);

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

float distanceFromDestination(sf::Vector2i source, sf::Vector2i destination)
{
	return std::abs(static_cast<float>(source.x - destination.x)) + static_cast<float>(std::abs(source.y - destination.y));
}

void getNeighbouringPoints(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours,
	const std::vector<std::vector<eCollidableTile>>& collisionLayer, sf::Vector2i levelSize)
{
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

sf::Vector2f PathFinding::getPositionClosestToTarget(sf::Vector2f source, sf::Vector2f target, const std::vector<std::vector<eCollidableTile>>& collisionLayer, sf::Vector2i levelSize, sf::Vector2i tileSize)
{
	resetGraph(levelSize, m_graph);

	std::queue<sf::Vector2i> frontier;
	frontier.push(sf::Vector2i(static_cast<int>(source.x / tileSize.x), static_cast<int>(source.y / tileSize.y)));

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	
}

bool PathFinding::isPositionReachable(sf::Vector2f source, sf::Vector2f target, const std::vector<std::vector<eCollidableTile>>& collisionLayer, 
	sf::Vector2i levelSize, sf::Vector2i tileSize)
{
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
		getNeighbouringPoints(lastPosition, neighbours, collisionLayer, levelSize);
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

void PathFinding::getPathToTile(sf::Vector2f source, sf::Vector2f destination, const std::vector<std::vector<eCollidableTile>>& collisionLayer, 
	sf::Vector2i levelSize, std::vector<sf::Vector2f>& pathToTile, sf::Vector2i tileSize)
{
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
		getNeighbouringPoints(lastPosition, neighbours, collisionLayer, levelSize);
		for (auto& neighbourPosition : neighbours)
		{
			if (collisionLayer[neighbourPosition.y][neighbourPosition.x] == eCollidableTile::eBox)
			{
				continue;
			}

			if (pathToTile.empty())
			{
				if (distanceFromDestination(neighbourPosition, positionAtDestination) < distanceFromDestination(positionAtSource, positionAtDestination))
				{
					m_graph[neighbourPosition.y][neighbourPosition.x] = GraphNode(lastPosition);
					pathToTile.emplace_back(neighbourPosition.x * tileSize.x, neighbourPosition.y * tileSize.y);
					frontier.push(neighbourPosition);
					break;
				}
			}
			else
			{
				if (distanceFromDestination(neighbourPosition, positionAtDestination) < distanceFromDestination(lastPosition, positionAtDestination))
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

void PathFinding::pathToClosestBox(sf::Vector2f source, const std::vector<std::vector<eCollidableTile>>& collisionLayer, 
	sf::Vector2i levelSize, std::vector<sf::Vector2f>& pathToTile, sf::Vector2i tileSize)
{
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
		getNeighbouringPoints(lastPosition, neighbours, collisionLayer, levelSize);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			if (collisionLayer[neighbourPosition.y][neighbourPosition.x] == eCollidableTile::eBox)
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
		int i = Utilities::getRandomNumber(0, static_cast<int>(boxSelection.size() - 1));
		lastPosition = m_graph[boxSelection[i].y][boxSelection[i].x].cameFrom;
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

void PathFinding::pathToClosestSafePosition(sf::Vector2f source, const std::vector<std::vector<eCollidableTile>>& collisionLayer,
	sf::Vector2i levelSize, std::vector<sf::Vector2f>& pathToTile, sf::Vector2i tileSize)
{
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
		getNeighbouringPoints(lastPosition, neighbours, collisionLayer, levelSize);
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