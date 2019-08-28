#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

struct FrontierNode
{
	FrontierNode(sf::Vector2f position, bool visited)
		: position(position),
		visited(visited)
	{}
	
	sf::Vector2f position;
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

private:

};