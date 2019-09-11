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

class Server;
enum class eCollidableTile;
class PathFinding
{
public:
	static PathFinding& getInstance()
	{
		static PathFinding instance;
		return instance;
	}

	void createGraph(sf::Vector2i levelSize);

	sf::Vector2f getPositionClosestToTarget(sf::Vector2f source, sf::Vector2f target, const Server& server);

	bool isPositionReachable(sf::Vector2f source, sf::Vector2f target, const Server& server);

	void getPathToTile(sf::Vector2f source, sf::Vector2f destination, std::vector<sf::Vector2f>& pathToTile, const Server& server);

	void pathToClosestBox(sf::Vector2f source, std::vector<sf::Vector2f>& pathToTile, const Server& server);

	void pathToClosestSafePosition(sf::Vector2f source, std::vector<sf::Vector2f>& pathToTile, const Server& server);

private:
	std::vector<std::vector<GraphNode>> m_graph;
};