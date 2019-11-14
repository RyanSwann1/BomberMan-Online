#pragma once

#include "NonCopyable.h"
#include <SFML/Graphics.hpp>
#include <vector>

struct GraphNode
{
	GraphNode();
	GraphNode(sf::Vector2i cameFrom);

	sf::Vector2i cameFrom;
	bool visited;
};

class Graph
{
public:
	Graph();

	void createGraph(sf::Vector2i levelSize);

	void resetGraph(sf::Vector2i levelSize, std::vector<std::vector<GraphNode>>& graph);
	void addToGraph(std::vector<std::vector<GraphNode>>& graph, sf::Vector2i position, sf::Vector2i levelSize);
	GraphNode getGraphNode(sf::Vector2i position, sf::Vector2i levelSize, const std::vector<std::vector<GraphNode>>& graph);

private:
	std::vector<std::vector<GraphNode>> m_graph;
};

class Server;
class PathFinding : private NonCopyable
{
public:
	static PathFinding& getInstance()
	{
		static PathFinding instance;
		return instance;
	}

	sf::Vector2f getPositionClosestToTarget(sf::Vector2f source, sf::Vector2f target, const Server& server);
	bool isPositionReachable(sf::Vector2f source, sf::Vector2f target, const Server& server);

	void createGraph(sf::Vector2i levelSize);

	void getPathToTile(sf::Vector2f source, sf::Vector2f destination, std::vector<sf::Vector2f>& pathToTile, const Server& server);
	void getPathToClosestBox(sf::Vector2f source, std::vector<sf::Vector2f>& pathToTile, const Server& server);
	void getPathToClosestPickUp(sf::Vector2f source, std::vector<sf::Vector2f>& pathToTile, const Server& server, int range);
	void getPathToClosestSafePosition(sf::Vector2f source, std::vector<sf::Vector2f>& pathToTile, const Server& server);

private:
	PathFinding() {}
	Graph m_graph;

	std::vector<sf::Vector2i> getPathToTile(sf::Vector2f source, sf::Vector2f destination, const Server& server);
};