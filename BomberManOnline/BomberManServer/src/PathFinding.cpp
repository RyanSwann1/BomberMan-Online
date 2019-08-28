#include "PathFinding.h"
#include "CollidableTile.h"
#include <math.h>

constexpr size_t MAX_NEIGHBOURS = 4;

float distanceFromSource(sf::Vector2f source, sf::Vector2f target)
{
	return std::abs(source.x - target.x) + std::abs(source.y - target.y);
}

void getNeighbours(sf::Vector2f position, std::vector<sf::Vector2f>& neighbours, 
	const std::vector<std::vector<eCollidableTile>>& collisionLayers)
{
	for (int x = position.x - 1; x <= position.x + 1; x += 2)
	{
		if (collisionLayers[position.y][x] == eCollidableTile::eNonCollidable)
		{
			neighbours.emplace_back(x, position.y);
		}
	}

	for (int y = position.y - 1; y <= position.y + 1; y += 2)
	{
		if (collisionLayers[position.x][y] == eCollidableTile::eNonCollidable)
		{
			neighbours.emplace_back(position.x, y);
		}
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
		col.reserve(21);
		row = col;
	}

	sf::Vector2f positionAtSource(source.x / 16, source.y / 16);

	//frontier.emplace_back(positionAtSource, true);

	std::vector<sf::Vector2f> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	//while (!frontier.empty())
	//{
	//	//getNeighbours(frontier.back().position, neighbours, collisionLayer);
	//	//for (auto& neighbour : neighbours)
	//	//{
	//	//	auto iter = std::find_if(frontier.cbegin(), frontier.cend() [)
	//	//}

	//}

	return std::vector<sf::Vector2f>();
}
