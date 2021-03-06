#pragma once

#include "NonCopyable.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <queue>

class GraphNode
{
public:
	GraphNode();
	GraphNode(sf::Vector2i cameFrom);

	sf::Vector2i getCameFrom() const;
	bool isVisited() const;

private:
	sf::Vector2i cameFrom;
	bool visited;
};

class Graph
{
public:
	Graph();

	bool isEmpty() const;
	sf::Vector2i getPreviousPosition(sf::Vector2i position, sf::Vector2i levelSize) const;
	bool isPositionVisited(sf::Vector2i position, sf::Vector2i levelSize) const;

	void resetGraph(sf::Vector2i levelSize);
	void addToGraph(sf::Vector2i position, sf::Vector2i lastPosition, sf::Vector2i levelSize);

private:
	std::vector<std::vector<GraphNode>> m_graph;
};

enum class eDirection;
class BombServer;
class Server;
class PathFinding : private NonCopyable
{
public:
	static PathFinding& getInstance()
	{
		static PathFinding instance;
		return instance;
	}

	bool isPositionReachable(sf::Vector2f sourcePosition, sf::Vector2f targetPosition, const Server& server);
	bool isPositionInRangeOfAllExplosions(sf::Vector2f sourcePosition, const Server& server);
	bool isPositionInRangeOfBombDetonation(sf::Vector2f sourcePosition, const BombServer& bomb, const Server& server);
	
	void createGraph(sf::Vector2i levelSize);

	void getNonCollidableAdjacentPositions(sf::Vector2f position, const Server& server, std::vector<sf::Vector2f>& positions);
	void getNonCollidableAdjacentPositions(sf::Vector2i position, const Server& server, sf::Vector2i ignorePosition, std::vector<sf::Vector2i>& positions);
	sf::Vector2f getFurthestNonCollidablePosition(sf::Vector2f sourcePosition, eDirection direction, const Server& server, int maxDistance) const;
	std::vector<sf::Vector2f> getPathToTile(sf::Vector2f sourcePosition, sf::Vector2f targetPosition, const Server& server); 
	void getPathToTile(sf::Vector2f sourcePosition, sf::Vector2f targetPosition, std::vector<sf::Vector2f>& pathToTile, const Server& server);
	void getSafePathToTile(sf::Vector2f sourcePosition, sf::Vector2f targetPosition, const BombServer& bomb, std::vector<sf::Vector2f>& pathToTile, const Server& server);

	void getSafePathToTile(sf::Vector2f sourcePosition, sf::Vector2f targetPosition, std::vector<sf::Vector2f>& pathToTile, const Server& erver);
	void getPathToClosestBox(sf::Vector2f sourcePosition, std::vector<sf::Vector2f>& pathToTile, const Server& server, int maxBoxOptions);
	void getPathToClosestPickUp(sf::Vector2f sourcePosition, std::vector<sf::Vector2f>& pathToTile, const Server& server, int range);
	void getPathToClosestSafePosition(sf::Vector2f sourcePosition, std::vector<sf::Vector2f>& pathToTile, const Server& server);
	void getPathToClosestSafePosition(sf::Vector2f sourcePosition, const BombServer& bomb, std::vector<sf::Vector2f>& pathToTile, const Server& server);
	void getPathToRandomLocalSafePosition(sf::Vector2f sourcePosition, std::vector<sf::Vector2f>& pathToTile, const Server& server, int maxPositionOptions);

private:
	PathFinding() {}
	Graph m_graph;
	std::queue<sf::Vector2i> m_frontier;
	std::vector<sf::Vector2i> m_adjacentPositions;

	std::vector<sf::Vector2f> getPathToVisitedTiles(sf::Vector2i sourcePosition, sf::Vector2i targetPosition, const Server& server);
	void getPathToVisitedTiles(sf::Vector2i sourcePosition, sf::Vector2i targetPosition, std::vector<sf::Vector2f>& pathToTile, const Server& server);
	void reset(sf::Vector2i levelSize);
};