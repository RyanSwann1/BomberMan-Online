#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

struct GraphNode
{
	GraphNode();
	GraphNode(sf::Vector2i cameFrom);

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

	void initGraph(sf::Vector2i levelSize);

	std::vector<sf::Vector2f> getPathToTile(sf::Vector2i source, sf::Vector2i destination, 
		const std::vector<std::vector<eCollidableTile>>& collisionLayer, sf::Vector2i levelSize);

	void pathToClosestBox(sf::Vector2i source, const std::vector<std::vector<eCollidableTile>>& collisionLayer, 
		sf::Vector2i levelSize, std::vector<sf::Vector2f>& pathToTile);

	void pathToClosestSafePosition(sf::Vector2i source, const std::vector<std::vector<eCollidableTile>>& collisionLayer,
		sf::Vector2i levelSize, std::vector<sf::Vector2f>& pathToTile);

private:
	std::vector<std::vector<GraphNode>> m_graph;
};