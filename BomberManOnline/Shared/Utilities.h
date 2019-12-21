#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

enum class eDirection;
enum class eCollidableTile;
namespace Utilities
{
	int distance(sf::Vector2f source, sf::Vector2f target, sf::Vector2i tileSize);
	int distance(sf::Vector2i source, sf::Vector2i target);

	bool isPositionNeighbouringBox(const std::vector<std::vector<eCollidableTile>>& collisionLayer, sf::Vector2f position, sf::Vector2i levelSize, sf::Vector2i tileSize);
	bool isPositionAdjacent(sf::Vector2f origin, sf::Vector2f neighbour, sf::Vector2i tileSize);

	sf::Vector2f Interpolate(sf::Vector2f pointA, sf::Vector2f pointB, float factor);
	sf::Vector2f getClosestGridPosition(sf::Vector2f position, sf::Vector2i tileSize);

	sf::Vector2i convertToGridPosition(sf::Vector2f position, sf::Vector2i tileSize);
	sf::Vector2f convertToWorldPosition(sf::Vector2i position, sf::Vector2i tileSize);

	bool traverseDirection(sf::Vector2f& position, sf::Vector2f endPosition, sf::Vector2i tileSize, eDirection direction);

	bool isPositionCollidable(const std::vector<std::vector<eCollidableTile>>& collisionLayer, sf::Vector2f position, sf::Vector2i tileSizedo);

	int getRandomNumber(int min, int max);
}