#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

struct FrontierNode
{
	FrontierNode(bool visited)
		: cameFrom(cameFrom),
		visited(visited)
	{}

	FrontierNode(bool visited, FrontierNode& cameFrom)
		: cameFrom(&cameFrom),
		visited(visited)
	{}
	
	FrontierNode* cameFrom;
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