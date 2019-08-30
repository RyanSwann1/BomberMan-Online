#include "PathFinding.h"
#include "CollidableTile.h"
#include "Utilities.h"
#include <math.h>
#include <queue>

constexpr size_t MAX_NEIGHBOURS = 4;
constexpr size_t MAX_BOX_SELECTION = 5;

void resetGraph(sf::Vector2i levelSize, std::vector<std::vector<GraphNode>>& graph);
bool isNodeVisited(const std::vector<std::vector<GraphNode>>& graph, sf::Vector2i position);
bool isNodeVisited(const std::vector<std::vector<bool>>& frontier, sf::Vector2i position);

void getNonCollidableNeighbours(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours,
	const std::vector<std::vector<eCollidableTile>>& collisionLayer);
void getNeighbours(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours,
	const std::vector<std::vector<eCollidableTile>>& collisionLayer);

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

bool isNodeVisited(const std::vector<std::vector<GraphNode>>& graph, sf::Vector2i position)
{
	return graph[position.y][position.x].visited;
}

bool isNodeVisited(const std::vector<std::vector<bool>>& frontier, sf::Vector2i position)
{
	return frontier[position.y][position.x];
}

float distanceFromDestination(sf::Vector2i source, sf::Vector2i destination)
{
	return std::abs(source.x - destination.x) + std::abs(source.y - destination.y);
}

void getNonCollidableNeighbours(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours,
	const std::vector<std::vector<eCollidableTile>>& collisionLayer)
{
	for (int x = position.x - 1; x <= position.x + 1; x += 2)
	{
		if (x >= 0 && x < 21 && collisionLayer[position.y][x] != eCollidableTile::eWall)
		{
			neighbours.emplace_back(x, position.y);
		}
	}

	for (int y = position.y - 1; y <= position.y + 1; y += 2)
	{
		if (y >= 0 && y < 21 && collisionLayer[y][position.x] != eCollidableTile::eWall)
		{
			neighbours.emplace_back(position.x, y);
		}
	}
}

void getNeighbours(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours, 
	const std::vector<std::vector<eCollidableTile>>& collisionLayer)
{
	for (int x = position.x - 1; x <= position.x + 1; x += 2)
	{
		if (x >= 0 && x < 21)
		{
			neighbours.emplace_back(x, position.y);
		}	
	}

	for (int y = position.y - 1; y <= position.y + 1; y += 2)
	{
		if (y >= 0 && y < 21)
		{
			neighbours.emplace_back(position.x, y);
		}
	}
}

void PathFinding::initGraph(sf::Vector2i levelSize)
{
	m_graph.resize(levelSize.y);
	for (auto& row : m_graph)
	{
		std::vector<GraphNode> col;
		col.resize(levelSize.x);
		row = col;
	}
}

std::vector<sf::Vector2f> PathFinding::getPathToTile(sf::Vector2i source, sf::Vector2i destination,
	const std::vector<std::vector<eCollidableTile>>& collisionLayer)
{
	std::vector<std::vector<bool>> frontier;
	frontier.resize(21);
	for (auto& row : frontier)
	{
		std::vector<bool> col;
		col.resize(21);
		row = col;
	}

	std::vector<sf::Vector2i> graph;
	graph.emplace_back(source);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	bool reachedDestination = false;
	while (!reachedDestination)
	{
		getNeighbours(graph.back(), neighbours, collisionLayer);
		for (auto& neighbourPosition : neighbours)
		{
			if (distanceFromDestination(neighbourPosition, destination) < distanceFromDestination(graph.back(), destination))
			{
				frontier[neighbourPosition.y][neighbourPosition.x] = true;
				graph.push_back(neighbourPosition);
				if (graph.back() == destination)
				{
					reachedDestination = true;
				}
			}
		}

		neighbours.clear();
	}

	int i = 0;

	return std::vector<sf::Vector2f>();
}

void PathFinding::pathToClosestBox(sf::Vector2i source, const std::vector<std::vector<eCollidableTile>>& collisionLayer, 
	sf::Vector2i levelSize, std::vector<sf::Vector2f>& pathToTile)
{
	resetGraph(levelSize, m_graph);

	std::queue<sf::Vector2i> frontier;
	frontier.push(source);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	std::vector<sf::Vector2i> boxSelection;
	boxSelection.reserve(MAX_BOX_SELECTION);
	sf::Vector2i lastPosition;
	bool boxFound = false;
	while (!frontier.empty())
	{
		lastPosition = frontier.front();
		frontier.pop();
		getNonCollidableNeighbours(lastPosition, neighbours, collisionLayer);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			if (collisionLayer[neighbourPosition.y][neighbourPosition.x] == eCollidableTile::eBox)
			{
				if (lastPosition == source)
				{
					continue;
				}

				if (boxSelection.size() < MAX_BOX_SELECTION)
				{
					m_graph[neighbourPosition.y][neighbourPosition.x] = GraphNode(lastPosition);
					boxSelection.push_back(neighbourPosition);
				}
				else
				{
					break;
					boxFound = true;
				}

				if (lastPosition == source)
				{
					pathToTile.emplace_back(lastPosition.x * 16, lastPosition.y * 16);
					boxFound = true;
					break;
				}
			}
			else
			{
				if (!isNodeVisited(m_graph, neighbourPosition))
				{
					m_graph[neighbourPosition.y][neighbourPosition.x] = GraphNode(lastPosition);
					frontier.push(neighbourPosition);
				}
			}
		}

		neighbours.clear();

		if (boxFound)
		{
			break;
		}
	}

	if (!boxSelection.empty())
	{
		int i = Utilities::getRandomNumber(0, static_cast<int>(boxSelection.size() - 1));
		lastPosition = m_graph[boxSelection[i].y][boxSelection[i].x].cameFrom;
		sf::Vector2i comeFrom = lastPosition;
		pathToTile.emplace_back(comeFrom.x * 16, comeFrom.y * 16);
		bool pathCompleted = false;
		while (!pathCompleted)
		{
			comeFrom = m_graph[comeFrom.y][comeFrom.x].cameFrom;
			if (comeFrom != source)
			{
				pathToTile.emplace_back(comeFrom.x * 16, comeFrom.y * 16);
			}
			else
			{
				pathCompleted = true;
			}
		}
	}
}

void PathFinding::pathToClosestSafePosition(sf::Vector2i source, const std::vector<std::vector<eCollidableTile>>& collisionLayer,
	sf::Vector2i levelSize, std::vector<sf::Vector2f>& pathToTile)
{
	resetGraph(levelSize, m_graph);

	std::queue<sf::Vector2i> frontier;
	frontier.push(source);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	bool safePositionFound = false;
	while (!frontier.empty())
	{
		sf::Vector2i lastPosition = frontier.front();
		frontier.pop();
		getNonCollidableNeighbours(lastPosition, neighbours, collisionLayer);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			if (collisionLayer[neighbourPosition.y][neighbourPosition.x] == eCollidableTile::eBox)
			{
				continue;
			}

			if (neighbourPosition.x != source.x && neighbourPosition.y != source.y)
			{
				pathToTile.emplace_back(neighbourPosition.x * 16, neighbourPosition.y * 16);
				pathToTile.emplace_back(lastPosition.x * 16, lastPosition.y * 16);
				safePositionFound = true;
				sf::Vector2i comeFrom = lastPosition;
				bool pathCompleted = false;
				while (!pathCompleted)
				{
					comeFrom = m_graph[comeFrom.y][comeFrom.x].cameFrom;
					if (comeFrom != source)
					{
						pathToTile.emplace_back(comeFrom.x * 16, comeFrom.y * 16);
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
				if (!isNodeVisited(m_graph, neighbourPosition))
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