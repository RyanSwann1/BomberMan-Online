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

	const std::vector<std::vector<GraphNode>>& getGraph() const;
	GraphNode getGraphNode(sf::Vector2i position, sf::Vector2i levelSize);

	void resetGraph(sf::Vector2i levelSize);
	void addToGraph(sf::Vector2i position, sf::Vector2i lastPosition, sf::Vector2i levelSize);

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

	void getPathToClosestBox(sf::Vector2f source, std::vector<sf::Vector2f>& pathToTile, const Server& server);
	void getPathToClosestPickUp(sf::Vector2f source, std::vector<sf::Vector2f>& pathToTile, const Server& server, int range);
	void getPathToClosestSafePosition(sf::Vector2f source, std::vector<sf::Vector2f>& pathToTile, const Server& server);

private:
	PathFinding() {}
	Graph m_graph;

	std::vector<sf::Vector2f> getPathToTile(sf::Vector2i neighbourPosition, sf::Vector2i lastPosition, const Server& server, 
		sf::Vector2i positionAtSource);
	void getPathtoTile(sf::Vector2i neighbourPosition, sf::Vector2i lastPosition, const Server& server,
		sf::Vector2i positionAtSource, std::vector<sf::Vector2f>& pathToTile);
};