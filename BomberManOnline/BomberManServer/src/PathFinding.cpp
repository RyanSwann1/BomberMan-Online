#include "PathFinding.h"
#include "CollidableTile.h"

void getNeighbours(sf::Vector2f position, std::vector<sf::Vector2f>& neighbours, 
	const std::vector<std::vector<eCollidableTile>>& collisionLayers)
{
	for (int x = position.x - 16; x <= position.x + 16; x += 16 * 2)
	{
		if (collisionLayers[position.y / 16][x / 16] == eCollidableTile::NonCollidable)
		{
			neighbours.emplace_back()
		}
	}

	for (int y = position.y - 16; y <= position.y + 16; y += 16 * 2)
	{

	}
}

std::vector<sf::Vector2f> PathFinding::getPathToTile(sf::Vector2f source, sf::Vector2f destination, 
	const std::vector<std::vector<eCollidableTile>>& collisionLayer)
{
	std::vector<FrontierNode> frontier;

	frontier.emplace_back(source, true);


	while (!frontier.empty())
	{

	}

}
