#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

struct GraphNode
{
	GraphNode()
		: cameFrom(),
		visited(false)
	{}

	GraphNode(sf::Vector2f cameFrom)
		: cameFrom(cameFrom),
		visited(true)
	{}

	sf::Vector2f cameFrom;
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

	std::vector<sf::Vector2f> getPathToTile(sf::Vector2f source, sf::Vector2f destination, 
		const std::vector<std::vector<eCollidableTile>>& collisionLayer);

	std::vector<sf::Vector2f> pathToClosestBox(sf::Vector2f source, const std::vector<std::vector<eCollidableTile>>& collisionLayer);

private:

};