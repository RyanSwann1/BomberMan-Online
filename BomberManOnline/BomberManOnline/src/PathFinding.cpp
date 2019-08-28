#include "PathFinding.h"
#include "CollidableTile.h"
#include <math.h>

constexpr size_t MAX_NEIGHBOURS = 4;

bool isNodeVisited(const std::vector<std::vector<FrontierNode>>& frontier, sf::Vector2f position)
{
	return frontier[position.y][position.x].visited;
}

bool isNodeVisited(const std::vector<std::vector<bool>>& frontier, sf::Vector2f position)
{
	return frontier[position.y][position.x];
}

float distanceFromDestination(sf::Vector2f source, sf::Vector2f destination)
{
	return std::abs(source.x - destination.x) + std::abs(source.y - destination.y);
}

void getNeighbours(sf::Vector2f position, std::vector<sf::Vector2f>& neighbours)
{
	for (int x = position.x - 1; x <= position.x + 1; x += 2)
	{
		neighbours.emplace_back(x, position.y);
	}

	for (int y = position.y - 1; y <= position.y + 1; y += 2)
	{
		neighbours.emplace_back(position.x, y);
	}
}

std::vector<sf::Vector2f> PathFinding::getPathToTile(sf::Vector2f source, sf::Vector2f destination, 
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

	std::vector<sf::Vector2f> graph;
	graph.emplace_back(source);

	std::vector<sf::Vector2f> neighbours;
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

std::vector<sf::Vector2f> PathFinding::pathToClosestBox(sf::Vector2f source, const std::vector<std::vector<eCollidableTile>>& collisionLayer)
{
	std::vector<std::vector<FrontierNode>> frontier;
	frontier.resize(21);
	for (auto& row : frontier)
	{
		std::vector<FrontierNode> col;
		col.resize(21);
		row = col;
	}

	std::vector<sf::Vector2f> graph;
	graph.emplace_back(source);
	sf::Vector2f lastPosition = source;

	std::vector<sf::Vector2f> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);
	
	bool boxFound = false;
	while (!boxFound)
	{
		getNeighbours(graph.back(), neighbours);
		for (auto& neighbourPosition : neighbours)
		{
			if (collisionLayer[neighbourPosition.y][neighbourPosition.x] == eCollidableTile::eBox)
			{
				boxFound = true;
			}
			else
			{
				if (!isNodeVisited(frontier, neighbourPosition))
				{
					frontier[neighbourPosition.y][neighbourPosition.x] = FrontierNode(true, lastPosition);
					break;
				}
			}
		}

		neighbours.clear();
	}

	return std::vector<sf::Vector2f>();
}
