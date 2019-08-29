#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

struct GraphNode
{
	GraphNode()
		: cameFrom(),
		visited(false)
	{}

	GraphNode(sf::Vector2i cameFrom)
		: cameFrom(cameFrom),
		visited(true)
	{}

	sf::Vector2i cameFrom;
	bool visited;
};

enum class eCollidableTile;
class PathFinding
{
public:
	static PathFinding& getInstance()
	{
		static PathFinding instance;
		return instance;
	}

	std::vector<sf::Vector2f> getPathToTile(sf::Vector2i source, sf::Vector2i destination, 
		const std::vector<std::vector<eCollidableTile>>& collisionLayer);

	std::vector<sf::Vector2f> pathToClosestBox(sf::Vector2i source, const std::vector<std::vector<eCollidableTile>>& collisionLayer);
	std::vector<sf::Vector2f> pathToClosestSafePosition(sf::Vector2i source, const std::vector<std::vector<eCollidableTile>>& collisionLayer);
};