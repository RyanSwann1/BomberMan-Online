#include "PathFinding.h"
#include "CollidableTile.h"
#include "Utilities.h"
#include "Server.h"
#include <math.h>
#include <queue>
#include <iostream>
#include <assert.h>

constexpr size_t MAX_NEIGHBOURS = 4;
constexpr size_t MAX_BOX_SELECTION = 5;

void getNonCollidableNeighbouringPoints(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours, const Server& server, sf::Vector2i ignorePosition);
void getNeighbouringPoints(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours, const Server& server);

void getNonCollidableNeighbouringPoints(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours, const Server& server, sf::Vector2i ignorePosition)
{
	for (int x = position.x - 1; x <= position.x + 1; x += 2)
	{
		if (x >= 0 && x < server.getLevelSize().x && server.getCollidableTile({ x, position.y }) != eCollidableTile::eWall
			&& server.getCollidableTile({ x, position.y }) != eCollidableTile::eBox &&
			sf::Vector2i(x, position.y) != ignorePosition)
		{
			neighbours.emplace_back(x, position.y);
		}
	}

	for (int y = position.y - 1; y <= position.y + 1; y += 2)
	{
		if (y >= 0 && y < server.getLevelSize().y && server.getCollidableTile({ position.x, y }) != eCollidableTile::eWall
			&& server.getCollidableTile({ position.x, y }) != eCollidableTile::eBox && 
			sf::Vector2i(position.x, y) != ignorePosition)
		{
			neighbours.emplace_back(position.x, y);
		}
	}
}

void getNeighbouringPoints(sf::Vector2i position, std::vector<sf::Vector2i>& neighbours, const Server& server)
{
	for (int x = position.x - 1; x <= position.x + 1; x += 2)
	{
		if (x >= 0 && x < server.getLevelSize().x && server.getCollidableTile({ x, position.y }) != eCollidableTile::eWall)
		{
			neighbours.emplace_back(x, position.y);
		}
	}

	for (int y = position.y - 1; y <= position.y + 1; y += 2)
	{
		if (y >= 0 && y < server.getLevelSize().y && server.getCollidableTile({ position.x, y }) != eCollidableTile::eWall)
		{
			neighbours.emplace_back(position.x, y);
		}
	}
}

//Graph Node
GraphNode::GraphNode()
	: cameFrom(),
	visited(false)
{}

GraphNode::GraphNode(sf::Vector2i cameFrom)
	: cameFrom(cameFrom),
	visited(true)
{}

Graph::Graph()
	: m_graph()
{
}

void Graph::addToGraph(sf::Vector2i position, sf::Vector2i lastPosition, sf::Vector2i levelSize)
{
	assert(position.x >= 0 || position.x < levelSize.x || position.y >= 0 || position.y < levelSize.y || !m_graph[position.y][position.y].visited);
	m_graph[position.y][position.x] = GraphNode(lastPosition);
}

const std::vector<std::vector<GraphNode>>& Graph::getGraph() const
{
	return m_graph;
}

GraphNode Graph::getGraphNode(sf::Vector2i position, sf::Vector2i levelSize)
{
	//assert(position.x < 0 || position.x > levelSize.x || position.y < 0 || position.y > levelSize.y);
	if (position.x >= 0 || position.x < levelSize.x || position.y >= 0 || position.y < levelSize.y)
	{
		return m_graph[position.y][position.x];
	}
}

void Graph::resetGraph(sf::Vector2i levelSize)
{
	if (m_graph.empty())
	{
		//Create
		m_graph.resize(levelSize.y);
		for (auto& row : m_graph)
		{
			std::vector<GraphNode> col;
			col.resize(levelSize.x);
			row = col;
		}
	}
	else
	{
		//Reset
		for (int y = 0; y < levelSize.y; y++)
		{
			for (int x = 0; x < levelSize.x; ++x)
			{
				m_graph[y][x] = GraphNode();
			}
		}
	}
}

sf::Vector2f PathFinding::getPositionClosestToTarget(sf::Vector2f source, sf::Vector2f target, const Server& server)
{
	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);
	sf::Vector2i tileSize = server.getTileSize();
	
	sf::Vector2i roundedTargetPosition(static_cast<int>(target.x / tileSize.x), static_cast<int>(target.y / tileSize.y));
	sf::Vector2i closestPosition = sf::Vector2i(static_cast<int>(source.x / tileSize.x), static_cast<int>(source.y / tileSize.y));
	getNeighbouringPoints(closestPosition, neighbours, server);

	for (sf::Vector2i neighbour : neighbours)
	{
		if (Utilities::distance(neighbour, roundedTargetPosition) < Utilities::distance(closestPosition, roundedTargetPosition))
		{
			closestPosition = neighbour;
		}
	}

	return sf::Vector2f(closestPosition.x * tileSize.x, closestPosition.y * tileSize.y);
}

bool PathFinding::isPositionReachable(sf::Vector2f source, sf::Vector2f target, const Server& server)
{
	sf::Vector2i levelSize = server.getLevelSize();
	sf::Vector2i tileSize = server.getTileSize();

	m_graph.resetGraph(levelSize);

	std::queue<sf::Vector2i> frontier;
	frontier.push(sf::Vector2i(static_cast<int>(source.x / tileSize.x), static_cast<int>(source.y / tileSize.y)));

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	sf::Vector2i lastPosition = sf::Vector2i(static_cast<int>(source.x / tileSize.x), static_cast<int>(source.y / tileSize.y));
	while (!frontier.empty())
	{
		lastPosition = frontier.front();
		frontier.pop();
		getNeighbouringPoints(lastPosition, neighbours, server);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			//Target found
			if (neighbourPosition == sf::Vector2i(static_cast<int>(target.x / tileSize.x), static_cast<int>(target.y / tileSize.y)))
			{
				return true;
			}
			else if (server.getCollidableTile(neighbourPosition) == eCollidableTile::eNonCollidable &&
				!m_graph.getGraphNode(neighbourPosition, server.getLevelSize()).visited)
			{
				m_graph.addToGraph(neighbourPosition, lastPosition, server.getLevelSize());
				frontier.push(neighbourPosition);
			}
		}
	}

	//Target Unreachable
	return false;
}

void PathFinding::createGraph(sf::Vector2i levelSize)
{
	assert(m_graph.getGraph().empty());
	m_graph.resetGraph(levelSize);
}

void PathFinding::getPathToClosestBox(sf::Vector2f source, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	pathToTile.clear();
	m_graph.resetGraph(server.getLevelSize());

	std::queue<sf::Vector2i> frontier;
	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i positionAtSource(source.x / tileSize.x, source.y / tileSize.y);
	frontier.push(positionAtSource);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	std::vector<sf::Vector2i> boxSelection;
	boxSelection.reserve(MAX_BOX_SELECTION);
	sf::Vector2i lastPosition;

	while (!frontier.empty() && boxSelection.size() < MAX_BOX_SELECTION)
	{
		lastPosition = frontier.front();
		frontier.pop();
		getNeighbouringPoints(lastPosition, neighbours, server);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			if(server.getCollidableTile(neighbourPosition) == eCollidableTile::eBox)
			{
				if (boxSelection.size() < MAX_BOX_SELECTION)
				{
					m_graph.addToGraph(neighbourPosition, lastPosition, server.getLevelSize());
					boxSelection.push_back(neighbourPosition);
				}
			}
			else
			{
				if (!m_graph.getGraphNode(neighbourPosition, server.getLevelSize()).visited)
				{
					m_graph.addToGraph(neighbourPosition, lastPosition, server.getLevelSize());
					frontier.push(neighbourPosition);
				}
			}
		}

		neighbours.clear();
	}

	if (!boxSelection.empty())
	{
		int randNumb = Utilities::getRandomNumber(0, static_cast<int>(boxSelection.size() - 1));
		lastPosition = m_graph.getGraphNode(sf::Vector2i(boxSelection[randNumb].x, boxSelection[randNumb].y), server.getLevelSize()).cameFrom;

		sf::Vector2i comeFrom = lastPosition;
		pathToTile.emplace_back(comeFrom.x * tileSize.x, comeFrom.y * tileSize.y);
		
		bool pathCompleted = false;
		while (!pathCompleted)
		{
			comeFrom = m_graph.getGraphNode(sf::Vector2i(comeFrom.x, comeFrom.y), server.getLevelSize()).cameFrom;
			if (comeFrom != positionAtSource)
			{
				pathToTile.emplace_back(comeFrom.x * tileSize.x, comeFrom.y * tileSize.y);
			}
			else
			{
				pathCompleted = true;
			}
		}
	}
}

void PathFinding::getPathToClosestPickUp(sf::Vector2f source, std::vector<sf::Vector2f>& pathToTile, const Server& server, int range)
{
	pathToTile.clear();
	m_graph.resetGraph(server.getLevelSize());

	sf::Vector2i tileSize = server.getTileSize();
	std::queue<sf::Vector2i> frontier;
	sf::Vector2i positionAtSource(source.x / tileSize.x, source.y / tileSize.y);
	frontier.push(positionAtSource);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	bool searchForPickUp = true;
	while (searchForPickUp && !frontier.empty())
	{
		sf::Vector2i lastPosition = frontier.front();
		frontier.pop();

		getNonCollidableNeighbouringPoints(lastPosition, neighbours, server, positionAtSource);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			if (!m_graph.getGraphNode(neighbourPosition, server.getLevelSize()).visited)
			{
				m_graph.addToGraph(neighbourPosition, lastPosition, server.getLevelSize());
				frontier.push(neighbourPosition);
				std::cout << neighbourPosition.x << " " << neighbourPosition.y << "\n";
			}

			if (getPathToTile(neighbourPosition, lastPosition, server, positionAtSource).size() 
				> range)
			{
				continue;
			}
			
			for (const GameObject& gameObject : server.getGameObjects())
			{
				if (gameObject.getPosition() == sf::Vector2f(neighbourPosition.x * tileSize.x, neighbourPosition.y * tileSize.y))
				{
					searchForPickUp = false;
					getPathtoTile(neighbourPosition, lastPosition, server, positionAtSource, pathToTile);
					//pathToTile.emplace_back(neighbourPosition.x * tileSize.x, neighbourPosition.y * tileSize.y);
					//pathToTile.emplace_back(lastPosition.x * tileSize.x, lastPosition.y * tileSize.y);
					//sf::Vector2i comeFrom = lastPosition;
					//bool pathCompleted = false;
					//while (!pathCompleted)
					//{
					//	comeFrom = m_graph.getGraphNode(comeFrom, server.getLevelSize()).cameFrom;
					//	if (comeFrom != positionAtSource)
					//	{
					//		pathToTile.emplace_back(comeFrom.x * tileSize.x, comeFrom.y * tileSize.y);
					//	}
					//	else
					//	{
					//		pathCompleted = true;
					//	}
					//}
					break;
				}
			}


		}

		neighbours.clear();
	}
}

void PathFinding::getPathToClosestSafePosition(sf::Vector2f source, std::vector<sf::Vector2f>& pathToTile, const Server& server)
{
	pathToTile.clear();
	m_graph.resetGraph(server.getLevelSize());

	sf::Vector2i tileSize = server.getTileSize();
	sf::Vector2i positionAtSource(source.x / tileSize.x, source.y / tileSize.y);
	
	std::queue<sf::Vector2i> frontier;
	frontier.push(positionAtSource);

	std::vector<sf::Vector2i> neighbours;
	neighbours.reserve(MAX_NEIGHBOURS);

	bool safePositionFound = false;
	while (!frontier.empty() && !safePositionFound)
	{
		sf::Vector2i lastPosition = frontier.front();
		frontier.pop();

		getNonCollidableNeighbouringPoints(lastPosition, neighbours, server, positionAtSource);
		for (sf::Vector2i neighbourPosition : neighbours)
		{
			if (neighbourPosition.x != positionAtSource.x && neighbourPosition.y != positionAtSource.y)
			{
				//getPathtoTile(neighbourPosition, lastPosition, server, positionAtSource, pathToTile);
				pathToTile.emplace_back(neighbourPosition.x * tileSize.x, neighbourPosition.y * tileSize.y);
				pathToTile.emplace_back(lastPosition.x * tileSize.x, lastPosition.y * tileSize.y);
				safePositionFound = true;
				sf::Vector2i comeFrom = lastPosition;
				bool pathCompleted = false;
				while (!pathCompleted)
				{
					comeFrom = m_graph.getGraphNode(comeFrom, server.getLevelSize()).cameFrom; 
					if (comeFrom != positionAtSource)
					{
						pathToTile.emplace_back(comeFrom.x * tileSize.x, comeFrom.y * tileSize.y);
					}
					else
					{
						pathCompleted = true;
					}
				}
				break;
			}
			else
			{
				if (!m_graph.getGraphNode(neighbourPosition, server.getLevelSize()).visited)
				{
					m_graph.addToGraph(neighbourPosition, lastPosition, server.getLevelSize());
					frontier.push(neighbourPosition);
				}
			}
		}

		neighbours.clear();
	}
}

std::vector<sf::Vector2f> PathFinding::getPathToTile(sf::Vector2i neighbourPosition, sf::Vector2i lastPosition, const Server& server, sf::Vector2i positionAtSource)
{
	//No path
	if (lastPosition == positionAtSource)
	{
		return std::vector<sf::Vector2f>();
	}

	std::vector<sf::Vector2f> pathToTile;

	sf::Vector2i tileSize = server.getTileSize();
	pathToTile.emplace_back(neighbourPosition.x* tileSize.x, neighbourPosition.y* tileSize.y);
	pathToTile.emplace_back(lastPosition.x* tileSize.x, lastPosition.y* tileSize.y);

	sf::Vector2i comeFrom = lastPosition;
	bool pathCompleted = false;
	while (!pathCompleted)
	{
		comeFrom = m_graph.getGraphNode(comeFrom, server.getLevelSize()).cameFrom;
		if (comeFrom != positionAtSource)
		{
			pathToTile.emplace_back(comeFrom.x * tileSize.x, comeFrom.y * tileSize.y);
		}
		else
		{
			pathCompleted = true;
		}
	}

	return pathToTile;
}

void PathFinding::getPathtoTile(sf::Vector2i neighbourPosition, sf::Vector2i lastPosition, const Server& server, sf::Vector2i positionAtSource, std::vector<sf::Vector2f>& pathToTile)
{
	//No path
	if (lastPosition == positionAtSource)
	{
		return;
	}

	sf::Vector2i tileSize = server.getTileSize();
	pathToTile.emplace_back(neighbourPosition.x * tileSize.x, neighbourPosition.y * tileSize.y);
	pathToTile.emplace_back(lastPosition.x * tileSize.x, lastPosition.y * tileSize.y);

	sf::Vector2i comeFrom = lastPosition;
	bool pathCompleted = false;
	while (!pathCompleted)
	{
		comeFrom = m_graph.getGraphNode(comeFrom, server.getLevelSize()).cameFrom;
		if (comeFrom != positionAtSource)
		{
			pathToTile.emplace_back(comeFrom.x * tileSize.x, comeFrom.y * tileSize.y);
		}
		else
		{
			pathCompleted = true;
		}
	}
}
